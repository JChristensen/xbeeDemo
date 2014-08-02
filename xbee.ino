//XBee demo for Flint Area Coding Meetup
//Jack Christensen 25Jul2014
//This work by Jack Christensen is licensed under CC BY-SA 4.0,
//http://creativecommons.org/licenses/by-sa/4.0/

//XBee variables
#define XBEE_PAYLOAD_LEN 40      //this cannot exceed the definition in the base station code
XBee xbee;                       //create the XBee object
union byteChar {byte B; char C;};
byteChar xbeePayload[XBEE_PAYLOAD_LEN], xbeeNI[9];
boolean xbWaitAck;               //waiting for an ack after transmitting a packet
XBeeAddress64 coordAddr(0x0, 0x0);
AtCommandResponse atResp;
ZBTxStatusResponse zbStat;
ZBRxResponse zbRX;
ZBTxRequest zbTX;
ModemStatusResponse zbMSR;
unsigned long msTX;              //last XBee transmission time

void sendData(void)
{
    xbeePayload[0].C = 'D';            //data packet
    xbeePayload[1].C = redState;
    xbeePayload[2].C = grnState;
    xbeePayload[3].B = servoAngle >> 8;
    xbeePayload[4].B = servoAngle;
    xbeePayload[5].C = 0;
    zbTX.setAddress64(coordAddr);       //build the tx request packet
    zbTX.setAddress16(0xFFFE);
    zbTX.setPayload(&xbeePayload[0].B);
    zbTX.setPayloadLength(6);
    xbee.send(zbTX);
    xbWaitAck = true;
    msTX = millis();
    Serial << endl << msTX << F(" XB TX") << endl;
}

//process the received data
void processData(void)
{
    digitalWrite(redLEDPin, xbeePayload[1].B);
    digitalWrite(grnLEDPin, xbeePayload[2].B);
    servoAngle = ( (uint16_t)xbeePayload[3].B << 8 ) + (uint16_t)xbeePayload[4].B;
    if (servoAngle < 8) servoAngle = 8;
    srvo.write(servoAngle);
}

void readXBee()                                       //process incoming traffic from the XBee
{
    byte xbeeResponse, respLen, *resp, delyStatus, dscyStatus, txRetryCount;
    uint32_t ms = millis();
    
    xbee.readPacket();
    if (xbee.getResponse().isAvailable()) {
        
        switch (xbee.getResponse().getApiId()) {      //what kind of packet did we get?

    	case ZB_TX_STATUS_RESPONSE:                   //transmit status for packets we've sent
            xbee.getResponse().getZBTxStatusResponse(zbStat);
            //get the delivery status, the fifth byte
            delyStatus = zbStat.getDeliveryStatus();
            dscyStatus = zbStat.getDiscoveryStatus();
            txRetryCount = zbStat.getTxRetryCount();
            switch (delyStatus) {
            case SUCCESS:
                xbWaitAck = false;
                Serial << ms << F(" XB ACK ") << ms - msTX << F("ms R=");
                Serial << txRetryCount << F(" DSCY=") << dscyStatus << endl;
                break;
            default:
             	Serial << F("XB TX ERR") << endl;         //the remote XBee did not receive our packet
                break;
            }
            break;

        case AT_COMMAND_RESPONSE:                          //response to NI or DB commands
            atResp = AtCommandResponse();
            xbee.getResponse().getAtCommandResponse(atResp);
            if (atResp.isOk()) {
                respLen = atResp.getValueLength();
                resp = atResp.getValue();
                for (int i=0; i<respLen; i++) {
                    xbeeNI[i].B = resp[i];
                }
                xbeeNI[respLen].B = '\0';                //assume 4-byte NI of the form XXnn
//                txSec = atoi(&xbeeNI[2].C);              //use nn to determine this node's transmit time
                Serial << ms << F(" XB NI=") << &xbeeNI[0].C << endl;
            }
            else {
                Serial << F("XB NI ERR") << endl;
            }
            break;

        case MODEM_STATUS_RESPONSE:                   //XBee administrative messages
            xbee.getResponse().getModemStatusResponse(zbMSR);
            xbeeResponse = zbMSR.getStatus();
            Serial << ms << ' ';
            switch (xbeeResponse) {
            case HARDWARE_RESET:
                Serial << F("XB HW RST") << endl;
                break;
            case ASSOCIATED:
                Serial << F("XB ASSOC") << endl;
                break;
            case DISASSOCIATED:
                Serial << F("XB DISASC") << endl;
                break;
            default:
                Serial << F("XB MDM STAT 0x") << _HEX(xbeeResponse) << endl;
                break;
            }            
            break;
    
        case ZB_RX_RESPONSE:                               //rx data packet
            xbee.getResponse().getZBRxResponse(zbRX);      //get the received data
            switch (zbRX.getOption()) {
            case ZB_PACKET_ACKNOWLEDGED:
                for (int i=0; i<XBEE_PAYLOAD_LEN; i++) {   //copy the received data to our buffer
                    xbeePayload[i].B = zbRX.getData(i);
                }
                //process the received data
                Serial << _DEC(ms) << F(" XB RX/ACK") << endl;
                switch (xbeePayload[0].C) {                //what type of packet
                case 'D':                                  //data packet
                    processData();
//                    Serial << F("XB RX data") << endl;
                    break;
                default:                                   //not expecting anything
                    Serial << F("XB unknown RX") << endl;
                    break;
                }
                break;
            default:
                Serial << F("XB RX no ACK") << endl;       //packet received and not ACKed
                break;
            }
            break;
        
        default:                                           //something else we were not expecting
            Serial << F("XB UNEXP TYPE") << endl;          //unexpected frame type
            break;
        }              
    }
}

