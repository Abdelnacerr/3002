
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdbool.h>
//#include <sys/types.h>

//#include <openssl/applink.c>
#include <openssl/bio.h>
#include <openssl/ssl.h> //create new ssl ctx
#include <openssl/err.h>

#define PORT 60000
#define BUFSIZE 1024
//                      2d pointer array
char *buildMessage(char *arrayToConvert[]) {

}


char *genQuestions(int numQuestions) {

    FILE *questionFD = fopen("questionsFileTest.txt", "r");

    if((questionFD) ==0) {
        perror("unable to open 1 or more of the required files");
        exit(EXIT_FAILURE);
    }
    //generate the questions
    int questionindexes[numQuestions];
    for (int i =0; i < numQuestions; i++) {
        //random numbers from 0 to 10
        questionindexes[numQuestions] = rand() % 11;
    }

//                              ***question length***
    char questionArray[numQuestions][100];
        for(int i = 0; i<numQuestions ; i++) {
            for(int j = 0; j <= questionindexes[i]; j++) {
                // read untill line reached
                //want to read the whole line and put it at questionArray[i], does fgets read char by char?
                if( fgets (questionArray[i][j], 60, questionFD)!=NULL );
            }
        }
    //malloc(sizeof(etc...)
    char *returnString = buildMessage(&questionArray);

    return returnString;
}


int markQuestion(char *message, int index) {
    return 0;
}

//clears the messageSent string so that the end of the message can be found (from seeing \0)
void clearSentMessage(char *message) {

}

//cycles through message until \0 to find its size (for efficiency)
int sizeOfSentMessage (char *message) {

}


//--------------------------ssl functions (move maybe) --------------------------------

void InitializeSSL()
{
    SSL_load_error_strings();
    SSL_library_init();
    OpenSSL_add_all_algorithms();
}

void DestroySSL()
{
    ERR_free_strings();
    EVP_cleanup();
}



//---------------------------------------------------------------------

int main(int argc, char *argv[]) {
    int sd_server;
    int sd_client;
    int sslSocketFD;
    char msgbuffer[BUFSIZE];// = {0}; // what is this 0 doing here
    char messageRecieved[BUFSIZE];
    char messageSent[BUFSIZE];

    int answer;
    int sentMessageSize;
    //do i need this struct?
    struct sockaddr_in address;
    int opt = 1; // NOT! unsued
    int addrlen = sizeof(address);

    SSL_CTX *sslContext;
    SSL *cSSL;
    
    //loads error strings, encryption functions, and the table with cyphers??
    InitializeSSL();
    
      
    // Creating socket descriptor
    sd_server = socket(AF_INET, SOCK_STREAM, 0);
    if (sd_server == 0) {
        perror("socket creation failed\n");
        exit(EXIT_FAILURE);
    }
      
    
    if (setsockopt(sd_server, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("socket options failed\n");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // binds the socket to the local host
    address.sin_port = htons( PORT );
      

    if (bind(sd_server, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("socket binding failed\n");
        exit(EXIT_FAILURE);
    }

    if (listen(sd_server, 3) < 0) {
        perror("listen failed\n");
        exit(EXIT_FAILURE);
    }

    while(true) {
        // check if this is right
        if ((sd_client = accept(sd_server, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
            perror("accept failed\n");
            exit(EXIT_FAILURE);
        }

        //----------------------ssl code stuff------------------------------

        sslContext = SSL_CTX_new( SSLv23_server_method());// is this the right server method?
        SSL_CTX_set_options(sslContext, SSL_OP_SINGLE_DH_USE); //adds the options set via bitmask in options to ctx ????

        //loads the certificate file into ctx
        //                                                 *need to make a server certificate*
        int use_cert = SSL_CTX_use_certificate_file(sslContext, "/serverCertificate.pem" , SSL_FILETYPE_PEM); 
        //adds the first private key found in the file to ctx
        int use_prv = SSL_CTX_use_PrivateKey_file(sslContext, "/serverCertificate.pem", SSL_FILETYPE_PEM);

        cSSL = SSL_new(sslContext);
        SSL_set_fd(cSSL, sd_client ); //connects the ssl object with a file descriptor
        //Here is the SSL Accept portion.  Now all reads and writes must use SSL
        sslSocketFD = SSL_accept(cSSL); //waits for a client to (TLS) handshake with the server (accepts incoming connection)
        if(sslSocketFD <= 0) {
            //Error occurred, log and close down ssl
            //accept failed
            perror("SSL handshake failed expectedly (0) or unexpectedly (>0)");
            SSL_shutdown(cSSL); //shutdown the TLS SSL connection
            SSL_free(cSSL); // free an allocated SSL structure
            exit(EXIT_FAILURE);
        }
     
        //--------------------------------------------------------------
        //                 does read read character by character??
        read(sd_client , buffer, 1024); // reads 1024 bytes from the socket file descriptor and puts it into buffer
        messageRecieved = buffer; // put the contents of the buffer into messageRecieved 


        //because using ssl reads and writes are done like:
        /*
        SSL_read(sd_client, buffer, 1024);
        SSL_write(sd_client, messageSent, 6); //6=numBytes to write (sizeof(messageSent)/sizeof(char))
        */

        if(messageRecieved[0] == 'Q') {
            printf("in here\n");
            messageSent = genQuestions(10); //usually get numQuestions from message
            
            //send the questions
            SSL_write(sd_client, messageSent, BUFSIZE); //find a better way of getting size!!!
            clearSentMessage(messageSent);
        } else if (messageRecieved[0] == 'M') {

            answer = markQuestion();
            sprintf(messageSent, "A;%d;%d", index, answer); //combine int answer into sent format
            SSL_write(sd_client, messageSent, BUFSIZE); //find a better way of getting size!!
            clearSentMessage(messageSent);

        } else if(messageRecieved[0] == 'E') {
            //end connection -- might not need this!!
        } else {
        
        }
    }
    return 0;
}
