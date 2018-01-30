/**
 * C++ Modbus RAW requests for very special moments!
 * Author: caffedrine@github
 *
 * Exception codes explained: http://www.simplymodbus.ca/exceptions.htm
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

// Error codes
#define ERR_ILLEGAL_FUNCTION        0x01
#define ERR_ILLEGAL_DATA_ADDRESS    0x02
#define ERR_ILLEGAL_DATA_VALUE      0x03
#define ERR_SLAVE_DEVICE_FAILURE    0x04
#define ERR_ACKNOWLEDGE             0x05
#define ERR_SLAVE_DEVICE_BUSY       0x06
#define ERR_NEGATIVE_ACKNOWLEDGE    0x07
#define ERR_MEMORY_PARITY_ERROR     0x08
#define ERR_GATEWAY_PATH_UNAVAILABLE    0x0A
#define ERR_GATEWAY_TARGET_DEVICE_FAILED_TO_RESPOND 0x0B

// Error messages
const char *err_msg[] = {
        /*NULL*/    "",
        /*0x01*/    "The function code received in the query is not an allowable action for the slave.  This may be because the function code is only applicable to newer devices, and was not implemented in the unit selected.  It could also indicate that the slave is in the wrong state to process a request of this type, for example because it is unconfigured and is being asked to return register values. If a Poll Program Complete command was issued, this code indicates that no program function preceded it.",
        /*0x02*/    "The data address received in the query is not an allowable address for the slave. More specifically, the combination of reference number and transfer length is invalid. For a controller with 100 registers, a request with offset 96 and length 4 would succeed, a request with offset 96 and length 5 will generate exception 02.",
        /*0x03*/    "A value contained in the query data field is not an allowable value for the slave.  This indicates a fault in the structure of remainder of a complex request, such as that the implied length is incorrect. It specifically does NOT mean that a data item submitted for storage in a register has a value outside the expectation of the application program, since the MODBUS protocol is unaware of the significance of any particular value of any particular register.",
        /*0x04*/    "An unrecoverable error occurred while the slave was attempting to perform the requested action.",
        /*0x05*/    "Specialized use in conjunction with programming commands.The slave has accepted the request and is processing it, but a long duration of time will be required to do so.  This response is returned to prevent a timeout error from occurring in the master. The master can next issue a Poll Program Complete message to determine if processing is completed.",
        /*0x06*/    "Specialized use in conjunction with programming commands.The slave is engaged in processing a long-duration program command.  The master should retransmit the message later when the slave is free..",
        /*0x07*/    "The slave cannot perform the program function received in the query. This code is returned for an unsuccessful programming request using function code 13 or 14 decimal. The master should request diagnostic or error information from the slave.",
        /*0x08*/    "Specialized use in conjunction with function codes 20 and 21 and reference type 6, to indicate that the extended file area failed to pass a consistency check.The slave attempted to read extended memory or record file, but detected a parity error in memory. The master can retry the request, but service may be required on the slave device.",
        /*NULL*/    "",
        /*0x0A*/    "Specialized use in conjunction with gateways, indicates that the gateway was unable to allocate an internal communication path from the input port to the output port for processing the request. Usually means the gateway is misconfigured or overloaded.",
        /*0x0B*/    "Specialized use in conjunction with gateways, indicates that no response was obtained from the target device. Usually means that the device is not present on the network."
};

int main()
{
    // Define connection variables
    int srcPort = 1337;
    int destPort = 1081;
    
    char srcAddr[] = "127.0.0.1";
    char destAddr[] = "127.0.0.1";
    
    // Socket variables
    socklen_t hSocket;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    
    // Resolve hostname if any
    server = gethostbyname(destAddr);
    if(server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(1);
    }
    
    // Initialize socket structures
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, sizeof(server->h_addr));
    serv_addr.sin_port = htons((uint16_t) destPort);
    
    // Initialize socket handler
    hSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(hSocket < 0)
    {
        fprintf(stderr, "Can't create socket handler...");
        exit(1);
    }
    
    // Connect to server
    if(connect(hSocket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        fprintf(stderr, "Can't connect to server...");
        exit(1);
    }
    
    // Modbus MBAP + PDU packet
    char modbus_packet[12];
    
    /// MBAP HEADER
    ////////////////////
    
    // Setting up tranzaction ID - two bytes
    modbus_packet[0] = 0x00;
    modbus_packet[1] = 0x01;
    
    // Setting up protocol ID - two bytes
    modbus_packet[2] = 0x00;
    modbus_packet[3] = 0x00;
    
    // Modbus length - two bytes
    modbus_packet[4] = 0x00;
    modbus_packet[5] = 0x06;
    
    // Setting up Unit ID
    modbus_packet[6] = 0x01;
    
    /// Setting up PDU
    ////////////////////
    
    // Modbus function code
    modbus_packet[7] = 0x01;
    
    // Modbus data start
    modbus_packet[8] = 0x00;
    modbus_packet[9] = 0x01;
    
    //Modbus data count
    modbus_packet[10] = 0x00;
    modbus_packet[11] = 0x0F;
    
    // Display packet you just created
    printf("-------------\nRequest RAW: ");
    for(int i = 0; i < 12; i++)
        printf("%02hhX ", modbus_packet[i]);
    printf("\n\n");
    
    // Some more debugging info to print
    printf("Transaction ID: %02hhX %02hhX\n", modbus_packet[0], modbus_packet[1]);
    printf("Protocol ID   : %02hhX %02hhX\n", modbus_packet[2], modbus_packet[3]);
    printf("Length        : %02hhX %02hhX\n", modbus_packet[4], modbus_packet[5]);
    printf("Unit ID       : %02hhX\n\n", modbus_packet[6]);
    printf("Function code : %02hhX\n", modbus_packet[7]);
    printf("Start address : %02hhX %02hhX\n", modbus_packet[8], modbus_packet[9]);
    printf("Data count    : %02hhX %02hhX\n\n", modbus_packet[10], modbus_packet[11]);
    
    // Write modbus packet
    ssize_t n = write(hSocket, modbus_packet, sizeof(modbus_packet));
    //write(hSocket, "\n", 1);
    if(n < 11)
    {
        fprintf(stderr, "Error while write......");
        exit(1);
    }
    
    // Read server response
    char buffer[128];       // 128 is enough since maximum 80 registers can be read via one request
    
    memset(buffer, 0, sizeof(buffer));
    n = read(hSocket, buffer, 128);
    if(n <= 0)
    {
        //TODO: Validate response message
        fprintf(stderr, "Error while read or not modbus present!");
        exit(1);
    }
    
    // Keep in mind when try to decode:
    // coils: 0 - 99999
    // registers: 400001 - 499999
    
    // Print raw response
    printf("-------------\nResponse RAW: ");
    for(int i = 0; i < n; i++)
        printf("%02hhX ", buffer[i]);
    printf("\n");
    
    if(buffer[7] != modbus_packet[7])
    {
        printf("Exception returned: %02hhX\n", buffer[8]);
        printf("Message: %s", err_msg[buffer[8]]);
        close(hSocket);
        exit(1);
    }
    
    // Display received data for debugging
    printf("%s\n", buffer);
    printf("Transaction ID: %02hhX %02hhX\n", buffer[0], buffer[1]);
    printf("Protocol ID   : %02hhX %02hhX\n", buffer[2], buffer[3]);
    printf("Length        : %02hhX %02hhX\n", buffer[4], buffer[5]);
    printf("Unit ID       : %02hhX\n", buffer[6]);
    printf("Function code : %02hhX\n", buffer[7]);
    printf("Data count    : %02hhX bytes\n\n", buffer[8]);
    
    close(hSocket);
    return 0;
}