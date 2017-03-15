
#include <project.h>
#include "stdio.h"
#include "stdbool.h"

#include "LED.h"

#define ERR_INVALID_PDU 0x04
#define CCCD_VALID_BIT_MASK 0x03
#define NOTIFY_BIT_MASK 0x01
#define FALSE           0
uint8 data[300] = {"CM:1""Dose""2 pills"}; //Initialisation of data

CYBLE_CONN_HANDLE_T connectionHandle;
CYBLE_GATTS_WRITE_REQ_PARAM_T *wrReqParam;
bool notificationsEnabled=false;
uint8 startNotification = 0;


int printstopper(char* buff,char success[],char fail[]){
    uint32 ch=WIFI_UartGetChar();
    uint32 i=0;
    uint8 e = '\n';
    int flag=0;
    int slen = strlen(success);
    int flen = strlen(fail);
    int ptr,eptr;
    while(1){
        while(ch==0u)ch=WIFI_UartGetChar();
        while(ch!=0u){
            buff[i++]=ch;
            ptr=i-slen-1;
            if((ptr>=0) & (buff[ptr]==e)){
                ptr++;
                eptr=ptr+slen;
                int j=0,flag=1;
                for(;ptr<eptr;ptr++)
                    if(buff[ptr]!=success[j++]){
                        flag=0;
                        break;
                    }
                if(flag){
                    return i;
                }
            }
            ptr=i-flen-1;
            if((ptr>=0) & (buff[ptr]==e)){
                ptr++;
                eptr=ptr+flen;
                int j=0,flag=1;
                for(;ptr<eptr;ptr++)
                    if(buff[ptr]!=fail[j++]){
                        flag=0;
                        break;
                    }
                if(flag){
                    return i;
                }
            }
            ch=WIFI_UartGetChar();
        }       
        if(flag)
            break;
    }
    return i;
}

char num2char(int a){
    switch(a){
        case 1:return '1';
        case 2:return '2';
        case 3:return '3';
        case 4:return '4';
        case 5:return '5';
        case 6:return '6';
        case 7:return '7';
        case 8:return '8';
        case 9:return '9';
        case 0:return '0';
        default:return '.';
    }
}

void printnum(uint32 num){
    if(num==0){
        UART_UartPutString("Number:");
        return;
    }
    char ch=num2char(num%10);
    printnum(num/10);
    UART_UartPutChar(ch);
}

void output(char suc[],char fa[]){
     char buff[5000];
     int len=printstopper(buff,suc,fa);
     int i;
     //printnum(i);
     for(i=0;i<len;i++)
         UART_UartPutChar(buff[i]);
}

void buff_print(uint8 buff[],int len){
    int i;
    for(i=0;i<len;i++)
         UART_UartPutChar(buff[i]);
    UART_UartPutChar('\n');
}

int get_field(char json[],int len,char name[],int nlen,uint8 value[]){
    int i=0,j=0,not_found=0;
    while(i<len){
        j=0;
        //compare field;
        not_found=0;
        i++;
        while(json[i]!='\"'){
            if(json[i]==name[j]){
                i++;
                j++;
                if(j>=nlen){
                    break;
                }
            }else{
                not_found=1;
                break;
            }
        }
        if(not_found){
           while(json[i]!=','&&i<len)i++;
           i++;
        }else{
            i+=3;
            int j=0;
            if(json[i-1]=='n'){
               i--; 
               while(json[i]!=',' && i<len)value[j++]=json[i++];
            }
            else
                while(json[i]!='\"' && i<len)value[j++]=json[i++];
            return j;
        }
    }
    return 0;
}

void wifihandler()
{
    uint8 d = '\r';
    uint8 e = '\n';
    uint8 s = '\"';
    
    UART_Start();
    WIFI_Start();
        
    WIFI_SpiUartClearRxBuffer();
    
    CyDelay(1000);
       
    char buff[5000],json[5000];
    
        WIFI_UartPutString("AT+CWJAP=");
        WIFI_UartPutChar(s);
        WIFI_UartPutString("SherlockedExt");
        WIFI_UartPutChar(s);
        WIFI_UartPutChar(',');
        WIFI_UartPutChar(s);
        WIFI_UartPutString("iamsherlocked");
        WIFI_UartPutChar(s);
        WIFI_UartPutChar(d);
        WIFI_UartPutChar(e);
        output("OK","ERROR");
    
        WIFI_UartPutString("AT+CIPMUX=0");
        WIFI_UartPutChar(d);
        WIFI_UartPutChar(e);
        output("OK","ERROR");
        
        //STARTING A TCP CONNECTION WITH THINGSPEAK
        WIFI_UartPutString("AT+CIPSTART=");
        WIFI_UartPutChar(s);
        WIFI_UartPutString("TCP");
        WIFI_UartPutChar(s);
        WIFI_UartPutChar(',');
        WIFI_UartPutChar(s);
        WIFI_UartPutString("api.thingspeak.com");
        WIFI_UartPutChar(s);
        WIFI_UartPutChar(',');
        WIFI_UartPutString("80");
        WIFI_UartPutChar(d);
        WIFI_UartPutChar(e);
        output("OK","ERROR");
            
        //SENDING THE COMMAND LENGTH
        WIFI_UartPutString("AT+CIPSEND=98");
        WIFI_UartPutChar(d);
        WIFI_UartPutChar(e);
        output(">","CLOSED");
               
        //SENDING THE COMMAND
        WIFI_UartPutString("GET /channels/173247/feeds.json?results=1 HTTP/1.1");
        WIFI_UartPutChar(d);
        WIFI_UartPutChar(e);
        WIFI_UartPutString("Host: api.thingspeak.com");
        WIFI_UartPutChar(d);
        WIFI_UartPutChar(e);
        WIFI_UartPutString("User-Agent: test");
        WIFI_UartPutChar(d);
        WIFI_UartPutChar(e);
        WIFI_UartPutChar(d);
        WIFI_UartPutChar(e);
        output("SEND OK","CLOSED");
        CyDelay(50);
        //output("CLOSED","CLOSED");
        //PARSING THE PARTICULAR NAME
         int len=printstopper(buff,"CLOSED","CLOSED");
        int end;
        for(end=len-1;end>0;end--){
            if(buff[end]=='}')
                break;
        }
        end-=2;
        int start=end-1;
        while(buff[start]!='{')
            start--;
        int i=0;
        start++;
        for(;start<end;start++)
            json[i++]=buff[start];
            int p;
       //buff_print(json,i);
        uint8 time1[10],time2[10],time3[10],dosage1[10],dosage2[10],dosage3[10];
        UART_UartPutChar(e);
        p=get_field(json,len,"field1",6,time1);
        buff_print(time1,p);
        p=get_field(json,len,"field2",6,dosage1);
        buff_print(dosage1,p);
        UART_UartPutChar(e);
        p=get_field(json,len,"field3",6,time2);
        buff_print(time2,p);
        UART_UartPutChar(e);
        p=get_field(json,len,"field4",6,dosage2);
        buff_print(dosage2,p);
        UART_UartPutChar(e);
        p=get_field(json,len,"field5",6,time3);
        buff_print(time3,p);
        UART_UartPutChar(e);
        p=get_field(json,len,"field6",6,dosage3);
        buff_print(dosage3,p);
        
        //STARTING A TCP CONNECTION WITH THINGSPEAK
        WIFI_UartPutString("AT+CIPSTART=");
        WIFI_UartPutChar(s);
        WIFI_UartPutString("TCP");
        WIFI_UartPutChar(s);
        WIFI_UartPutChar(',');
        WIFI_UartPutChar(s);
        WIFI_UartPutString("api.thingspeak.com");
        WIFI_UartPutChar(s);
        WIFI_UartPutChar(',');
        WIFI_UartPutString("80");
        WIFI_UartPutChar(d);
        WIFI_UartPutChar(e);
        output("OK","ERROR");
        
        //SENDING THE COMMAND LENGTH
        WIFI_UartPutString("AT+CIPSEND=98");
        WIFI_UartPutChar(d);
        WIFI_UartPutChar(e);
        output(">","CLOSED");
               
        //SENDING THE COMMAND
        WIFI_UartPutString("GET /channels/173248/feeds.json?results=1 HTTP/1.1");
        WIFI_UartPutChar(d);
        WIFI_UartPutChar(e);
        WIFI_UartPutString("Host: api.thingspeak.com");
        WIFI_UartPutChar(d);
        WIFI_UartPutChar(e);
        WIFI_UartPutString("User-Agent: test");
        WIFI_UartPutChar(d);
        WIFI_UartPutChar(e);
        WIFI_UartPutChar(d);
        WIFI_UartPutChar(e);
        output("SEND OK","CLOSED");
        CyDelay(50);

        //PARSING THE PARTICULAR NAME
        len=printstopper(buff,"CLOSED","CLOSED");
        for(end=len-1;end>0;end--){
            if(buff[end]=='}')
                break;
        }
        end-=2;
         start=end-1;
        while(buff[start]!='{')
            start--;
         i=0;
        start++;
        for(;start<end;start++)
            json[i++]=buff[start];
        
        //buff_print(json,i);
        uint8 time4[10],time5[10],time6[10],dosage4[10],dosage5[10],dosage6[10];
        UART_UartPutChar(e);
        p=get_field(json,len,"field1",6,time4);
        buff_print(time4,p);
        p=get_field(json,len,"field2",6,dosage4);
        buff_print(dosage4,p);
        UART_UartPutChar(e);
        p=get_field(json,len,"field3",6,time5);
        buff_print(time5,p);
        UART_UartPutChar(e);
        p=get_field(json,len,"field4",6,dosage5);
        buff_print(dosage5,p);
        UART_UartPutChar(e);
        p=get_field(json,len,"field5",6,time6);
        buff_print(time6,p);
        UART_UartPutChar(e);
        p=get_field(json,len,"field6",6,dosage6);
        buff_print(dosage6,p);       
                                          
        //STARTING A TCP CONNECTION WITH THINGSPEAK
        WIFI_UartPutString("AT+CIPSTART=");
        WIFI_UartPutChar(s);
        WIFI_UartPutString("TCP");
        WIFI_UartPutChar(s);
        WIFI_UartPutChar(',');
        WIFI_UartPutChar(s);
        WIFI_UartPutString("api.thingspeak.com");
        WIFI_UartPutChar(s);
        WIFI_UartPutChar(',');
        WIFI_UartPutString("80");
        WIFI_UartPutChar(d);
        WIFI_UartPutChar(e);
        output("OK","ERROR");
        
        //SENDING THE COMMAND LENGTH
        WIFI_UartPutString("AT+CIPSEND=98");
           WIFI_UartPutChar(d);
        WIFI_UartPutChar(e);
        output(">","CLOSED");
               
        //SENDING THE COMMAND
        WIFI_UartPutString("GET /channels/173250/feeds.json?results=1 HTTP/1.1");
        WIFI_UartPutChar(d);
        WIFI_UartPutChar(e);
        WIFI_UartPutString("Host: api.thingspeak.com");
        WIFI_UartPutChar(d);
        WIFI_UartPutChar(e);
        WIFI_UartPutString("User-Agent: test");
        WIFI_UartPutChar(d);
        WIFI_UartPutChar(e);
        WIFI_UartPutChar(d);
        WIFI_UartPutChar(e);
        output("SEND OK","CLOSED");
        CyDelay(50);
        //output("CLOSED","CLOSED");
        //PARSING THE PARTICULAR NAME
         len=printstopper(buff,"CLOSED","CLOSED");
        
        for(end=len-1;end>0;end--){
            if(buff[end]=='}')
                break;
        }
        end-=2;
         start=end-1;
        while(buff[start]!='{')
            start--;
        i=0;
        start++;
        for(;start<end;start++)
            json[i++]=buff[start];
            
       //buff_print(json,i);
        uint8 time7[10],time8[10],time9[10],dosage7[10],dosage8[10],dosage9[10];
        UART_UartPutChar(e);
        p=get_field(json,len,"field1",6,time7);
        buff_print(time7,p);
        p=get_field(json,len,"field2",6,dosage7);
        buff_print(dosage7,p);
        UART_UartPutChar(e);
        p=get_field(json,len,"field3",6,time8);
        buff_print(time8,p);
        UART_UartPutChar(e);
        p=get_field(json,len,"field4",6,dosage8);
        buff_print(dosage8,p);
        UART_UartPutChar(e);
        p=get_field(json,len,"field5",6,time9);
        buff_print(time9,p);
        UART_UartPutChar(e);
        p=get_field(json,len,"field6",6,dosage9);
        buff_print(dosage9,p);
                                                
        //STARTING A TCP CONNECTION WITH THINGSPEAK
        WIFI_UartPutString("AT+CIPSTART=");
        WIFI_UartPutChar(s);
        WIFI_UartPutString("TCP");
        WIFI_UartPutChar(s);
        WIFI_UartPutChar(',');
        WIFI_UartPutChar(s);
        WIFI_UartPutString("api.thingspeak.com");
        WIFI_UartPutChar(s);
        WIFI_UartPutChar(',');
        WIFI_UartPutString("80");
        WIFI_UartPutChar(d);
        WIFI_UartPutChar(e);
        output("OK","ERROR");
        
        //SENDING THE COMMAND LENGTH
        WIFI_UartPutString("AT+CIPSEND=98");
           WIFI_UartPutChar(d);
        WIFI_UartPutChar(e);
        output(">","CLOSED");
               
        //SENDING THE COMMAND
        WIFI_UartPutString("GET /channels/173252/feeds.json?results=1 HTTP/1.1");
        WIFI_UartPutChar(d);
        WIFI_UartPutChar(e);
        WIFI_UartPutString("Host: api.thingspeak.com");
        WIFI_UartPutChar(d);
        WIFI_UartPutChar(e);
        WIFI_UartPutString("User-Agent: test");
        WIFI_UartPutChar(d);
        WIFI_UartPutChar(e);
        WIFI_UartPutChar(d);
        WIFI_UartPutChar(e);
        output("SEND OK","CLOSED");
        CyDelay(50);
        //output("CLOSED","CLOSED");
        //PARSING THE PARTICULAR NAME
         len=printstopper(buff,"CLOSED","CLOSED");
        for(end=len-1;end>0;end--){
            if(buff[end]=='}')
                break;
        }
        end-=2;
        start=end-1;
        while(buff[start]!='{')
            start--;
        i=0;
        start++;
        for(;start<end;start++)
            json[i++]=buff[start];
       //buff_print(json,i);
        uint8 time10[10],time11[10],time12[10],dosage10[10],dosage11[10],dosage12[10];
        UART_UartPutChar(e);
        p=get_field(json,len,"field1",6,time10);
        buff_print(time10,p);
        p=get_field(json,len,"field2",6,dosage10);
        buff_print(dosage10,p);
        UART_UartPutChar(e);
        p=get_field(json,len,"field3",6,time11);
        buff_print(time11,p);
        UART_UartPutChar(e);
        p=get_field(json,len,"field4",6,dosage11);
        buff_print(dosage11,p);
        UART_UartPutChar(e);
        p=get_field(json,len,"field5",6,time12);
        buff_print(time12,p);
        UART_UartPutChar(e);
        p=get_field(json,len,"field6",6,dosage12);
        buff_print(dosage12,p);
//        return 0;      
    
}

void StackEventHandler(uint32 event,void *eventParam)
{
    
    CYBLE_GATTS_WRITE_REQ_PARAM_T *wrReqParam;

    switch(event)
	{
        
     
	case CYBLE_EVT_STACK_ON:         
	            CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
		    UART_UartPutString("Stack is on");
		    GREEN_LED_ON();
            	    break;		
	case CYBLE_EVT_STACK_BUSY_STATUS:
		   UART_UartPutString("Stack is busy");
		   break;
	case CYBLE_EVT_TIMEOUT:
		   UART_UartPutString("Timeout");	    
		   break;
        case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:

            CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
            UART_UartPutString("Advertising");
	    CyDelay(1500);
	    ALL_LED_OFF();
            break;

	case CYBLE_EVT_GATT_CONNECT_IND:            
            connectionHandle = *(CYBLE_CONN_HANDLE_T *) eventParam;
            break;

        case CYBLE_EVT_GAP_DEVICE_CONNECTED:
            /* BLE link is established */
            BLUE_LED_ON();
            UART_UartPutString("connected");
            printnum(event);            
            break;

        case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
		UART_UartPutString("ADB StartStop");
		printnum(CyBle_GetState());
	 if(CyBle_GetState() == CYBLE_STATE_DISCONNECTED)
            {
		
                UART_UartPutString("NOT CONNECTED");
		CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
                RED_LED_ON();
            
                            }
            	break;

        /* Other events that are generated by the BLE Component that
         * are not required for functioning of this design */
        /**********************************************************
        *                       General Events
        ***********************************************************/
        case CYBLE_EVT_HARDWARE_ERROR:    /* This event indicates that some internal HW error has occurred. */
            break;

        case CYBLE_EVT_HCI_STATUS:
            break;

        /**********************************************************
        *                       GAP Events
        ***********************************************************/
        case CYBLE_EVT_GAP_AUTH_REQ:
            break;

        case CYBLE_EVT_GAP_PASSKEY_ENTRY_REQUEST:
            break;

        case CYBLE_EVT_GAP_PASSKEY_DISPLAY_REQUEST:
            break;

        case CYBLE_EVT_GAP_AUTH_COMPLETE:

            break;
        case CYBLE_EVT_GAP_AUTH_FAILED:

            break;

        case CYBLE_EVT_GAP_ENCRYPT_CHANGE:
            break;

        case CYBLE_EVT_GAPC_CONNECTION_UPDATE_COMPLETE:
            break;

        /**********************************************************
        *                       GATT Events
        ***********************************************************/
        
        case CYBLE_EVT_GATT_DISCONNECT_IND:
            
            startNotification = 0;
			/* Red LED Glows when device is disconnected */
            RED_LED_ON();
            break;    

        case CYBLE_EVT_GATTS_XCNHG_MTU_REQ:
            break;

        case CYBLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ:
            break;
        
        case CYBLE_EVT_GATTS_WRITE_REQ:
            
            UART_UartPutString ("GATT_WRITE_REQ received\r\n");
            wrReqParam = (CYBLE_GATTS_WRITE_REQ_PARAM_T *) eventParam;
            if(CYBLE_SENDDATA_DATA_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE == wrReqParam->handleValPair.attrHandle)
                {
                    if(FALSE == (wrReqParam->handleValPair.value.val[CYBLE_SENDDATA_DATA_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_INDEX] & (~CCCD_VALID_BIT_MASK)))
                        {
                            startNotification = wrReqParam->handleValPair.value.val[0];
                            UART_UartPutString ("%d notification\r\n");
                            CyBle_GattsWriteAttributeValue(&wrReqParam->handleValPair,FALSE, &cyBle_connHandle, CYBLE_GATT_DB_LOCALLY_INITIATED);
                        }
                     else
                        {
                            UART_UartPutString ("error GATT_WRITE\r\n");
                            CYBLE_GATTS_ERR_PARAM_T err_param;
                            err_param.opcode = CYBLE_GATT_WRITE_REQ;
                            err_param.attrHandle = wrReqParam->handleValPair.attrHandle;
                            err_param.errorCode = ERR_INVALID_PDU;
                            (void)CyBle_GattsErrorRsp(cyBle_connHandle, &err_param);
                            return;
                        }
                }
            CyBle_GattsWriteRsp(cyBle_connHandle);
        break;

          

        /**********************************************************
        *                       Other Events
        ***********************************************************/
        case CYBLE_EVT_PENDING_FLASH_WRITE:
            break;

        default:
            break;
    }
}

int main()
{
    CYBLE_API_RESULT_T apiResult;
    //ALL_LED_OFF ();  
   
    CyGlobalIntEnable;  /* Comment this line to disable global interrupts. */
    
    /* Start BLE component and register Event handler function */	
    //wifihandler();
    apiResult = CyBle_Start(StackEventHandler);
    printnum(apiResult);
    while(1)
    {
            CYBLE_GATTS_HANDLE_VALUE_NTF_T notificationHandle;
        
	    CyBle_ProcessEvents();   
        
            if(CyBle_GetState() == CYBLE_STATE_CONNECTED){
		UART_UartPutString("Connected");
		    CyBle_ProcessEvents();
		GREEN_LED_ON();
		ALL_LED_OFF();		
		}
            if(CyBle_GetState() == CYBLE_STATE_DISCONNECTED)
		{
		UART_UartPutString("NOT CONNECTED!!");

		//CyDelay(2000);		
                RED_LED_ON();
		//CyDelay(3000);
		CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);		
		GREEN_LED_ON();
		CyBle_ProcessEvents();

		}
	    if(startNotification & NOTIFY_BIT_MASK)
            	{
                 
                notificationHandle.attrHandle = CYBLE_SENDDATA_DATA_CHAR_HANDLE;
                notificationHandle.value.val = &data;
                notificationHandle.value.len = 15;
                
                apiResult=   CyBle_GattsNotification(cyBle_connHandle, &notificationHandle);
                if (apiResult != CYBLE_ERROR_OK)
                    {
                        printf ("Sending Data/Command failed\r\n");
                    }
                    else
                    {
                        printf ("Command / Data sent\r\n");
                    }
                 }        
        }

wifihandler();
    }


/* [] END OF FILE */
