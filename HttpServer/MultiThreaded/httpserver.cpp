#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <err.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUF_SIZE 40000
#define HEAD_BUF_SIZE 4000
#define PORT 8080
#define CODE_100 "100-continue"
#define CODE_200 "200 OK"
#define CODE_201 "201 Created"
#define CODE_400 "400 Bad_Request"
#define CODE_403 "403 Forbidden"
#define CODE_404 "404 Not_Found"
#define CODE_500 "500 Internal_Server_Error"

void *dispatch_thread_function(void* dummy);
void *thread_func(void* dummy);
void Log_Fail_Write(char* request, char* filename, int code);


pthread_mutex_t mutex_file = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_buffer = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_log = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t temporary_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t condition_var = PTHREAD_MUTEX_INITIALIZER;

int sockfd;
struct sockaddr_in addr_in;
int len = sizeof(addr_in);
int shared_buffer[100];
int store_index = 0;
int retrieve_index = 0;
int waiting = 0;
int log_flag = 0;
int log_fd;

int main(int argv, char** argc){

   // char* buffer = (char*)malloc(BUF_SIZE);
    char* data = (char*)malloc(BUF_SIZE);
    
   // struct stat for_size;
   // char delimit[] = " \r\n";
  //  char delimit_slash[] = " /\r\n";
    //char valid[] = "qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM-_1234567890";
    char* request = (char*)malloc(5);
    char* out_header = (char*)malloc(BUF_SIZE);
    char* protocol = (char*)malloc(20);
    char* status = (char*)malloc(20);
    char* token = (char*)malloc(100);
    char* log_filename = (char*)malloc(28 * sizeof(char));

    int thread_num = 4;

    for(int x = 1; x < argv; x++){
        if(strcmp(argc[x],"-l") == 0 && (x+1) < argv){
            log_filename = argc[x+1];
            log_fd = open(log_filename, O_RDONLY  | O_TRUNC | O_WRONLY | O_CREAT);
            fchmod(log_fd, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
            log_flag = 1;
        }
        else if(strcmp(argc[x],"-N") == 0 && (x+1) < argv){
            thread_num = atoi(argc[x+1]);
        }
    }

    printf("Number of threads: %d, log filename: %s \n", thread_num, log_filename);

    pthread_t dispatch_thread;
    pthread_t thread_id[thread_num + 1];

    //pthread_create(&dispatch_thread, NULL, dispatch_thread_function, NULL);

    //for(int x = 0; x < thread_num; x++){
     //   pthread_create(&thread_id[x], NULL, thread_func, NULL);
    //} 







    //int file_length = 0;
    int opt = 1;
    //int sockfd;
   // int new_sock, valread;
    char* filename = (char*)malloc(28 * sizeof(char));
    //struct sockaddr_in addr_in;
   // int len = sizeof(addr_in);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == 0){
        perror("socket failed \n");
        exit(EXIT_FAILURE);
    }
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    addr_in.sin_family = AF_INET;
    addr_in.sin_addr.s_addr = INADDR_ANY;
  /*  if(argv >1){//checks arguments for ip address and port
        if(argv == 3){
        addr_in.sin_port = htons(atoi(argc[2]));
        }
        else{
            addr_in.sin_port = htons(PORT);
        }
         inet_aton(argc[1], &addr_in.sin_addr);
         
         //printf("%s %s\n", argc[1], argc[2]);
    }
    else if(argv >= 4){
        perror("Too many cmd arguments");
        exit(EXIT_FAILURE);
    }
    else{//default port 8080 and ip address 127.0.0.1
    */
    addr_in.sin_port = htons(PORT);

    inet_aton("127.0.0.1", &addr_in.sin_addr);

    for(int x = 1; x < argv; x++){
        if(strcmp(argc[x],"-P") == 0 && (x+1) < argv){
            addr_in.sin_port = htons(atoi(argc[x+1]));
            printf("Port: %d\n", atoi(argc[x+1]));
        }
        else if(strcmp(argc[x],"-A") == 0 && (x+1) < argv){
            inet_aton(argc[x+1], &addr_in.sin_addr);
            printf("Addr: %s\n", argc[x+1]);
        }
    }

    if(bind(sockfd, (struct sockaddr *) &addr_in, sizeof(addr_in)) < 0){
        perror("bind failed");
        exit(EXIT_FAILURE);

    }

    if(listen(sockfd, 5) < 0){
        perror("listen");
        exit(EXIT_FAILURE);
    }

    /*space for threads 

    

    */

     pthread_mutex_lock(&condition_var);
    pthread_create(&dispatch_thread, NULL, dispatch_thread_function, NULL);

    for(int x = 0; x < thread_num; x++){
        pthread_create(&thread_id[x], NULL, thread_func, NULL);
    } 


    while(1){
    }
    

    free(data);
    free(out_header);
    free(protocol);
    free(status);
    free(request);
    free(token);
    free(filename);
    free(log_filename);
    return 1;
    
}

//int reserved_space = (length * 3) + (9 * (length % 20));

void Log_Fail_Write(char* request, char* filename, int code){
    
    char* out_header = (char*)malloc(BUF_SIZE);
    snprintf(out_header, BUF_SIZE, "FAIL: %s %s HTTP/1.1 --- response %d\n========\n", request, filename, code);
    write(log_fd, out_header, strlen(out_header));

    free(out_header);
    
}

void *dispatch_thread_function(void* dummy){
    //struct stat for_size;
   // char delimit[] = " \r\n";
   // char delimit_slash[] = " /\r\n";
   // char valid[] = "qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM-_1234567890";
    char* request = (char*)malloc(5);
    char* out_header = (char*)malloc(BUF_SIZE);

    
    while(1){

   // int file_length = 0;
    int new_sock = accept(sockfd, (struct sockaddr *) &addr_in, (socklen_t *) &len);
    if(new_sock < 0){//checks if valid socket
        perror("accept");
        exit(EXIT_FAILURE);
    }
    
    while(shared_buffer[store_index] > 0){
    }
    while(waiting <= 0){    
    }
    shared_buffer[store_index] = new_sock;
    pthread_mutex_unlock(&condition_var);
    printf("Post unlock\n");
    waiting--;
    
    
    store_index = (++store_index) % 10;
    printf("end of loop\n");
    }

    free(out_header);
    free(request);

    
}

void *thread_func(void* dummy){
    char* buffer = (char*)malloc(BUF_SIZE);
    char* data = (char*)malloc(BUF_SIZE);
    
    struct stat for_size;
    char delimit[] = " \r\n";
    char delimit_slash[] = " /\r\n";
    char valid[] = "qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM-_1234567890";
    char* request = (char*)malloc(5);
    char* out_header = (char*)malloc(BUF_SIZE);
    char* protocol = (char*)malloc(20);
    char* status = (char*)malloc(20);
    char* token = (char*)malloc(100);

    int  valread;
    char* filename = (char*)malloc(28 * sizeof(char));
    char* log_buff = (char*)malloc(75);
    
    int new_sock;
    while(1){
        //printf("beginning of thread loop\n");
        waiting++;
        
        //pthread_cond_wait(&condition_var, &mutex_buffer);
        pthread_mutex_lock(&condition_var);
       // printf("number waiting %d \n", waiting);
        pthread_mutex_lock(&temporary_mutex);
        new_sock = shared_buffer[retrieve_index];//retrieve socket from buffer
        shared_buffer[retrieve_index] = -1;//resets the buffer space       
        retrieve_index = (++retrieve_index) % 10;//increase index
        pthread_mutex_unlock(&temporary_mutex);
        //pthread_mutex_unlock(&mutex_buffer);
      // printf("after unlock \n");
        //
        //Parse request here
        memset(data, 0, BUF_SIZE); //resets buffers
        memset(buffer,0, BUF_SIZE);
        int file_length = 0;

        int invalid = 0;
        int no_length = 0;
       // printf("pre Valread \n");

       // printf("before recv\n");
        valread = recv(new_sock, buffer, BUF_SIZE, 0);
        //printf("after recv\n");
        //printf("data received, buffer: %s \n", buffer);

        if(strstr(buffer, "Content-Length:") == NULL){//if buffer does not contain Content-Length: set flag
            no_length = 1;
        }
       // printf("length check \n");
        request = strtok(buffer, delimit);
        filename = strtok(NULL, delimit_slash);
        //printf("filename: %s, request: %s \n", filename, request);
        for(size_t x = 0; x < strlen(filename);x++){
            int flag = 0;
            for(size_t y = 0; y < strlen(valid); y++){
                if(filename[x] == valid[y]){
                    flag = 1;//checks if each character is a valid character
                }
            }
            if(flag == 0){
                invalid = 1;
                break;
            }
        }


        pthread_mutex_lock(&mutex_file);
        //Perform request

        if(invalid == 1){//if invalid character, status code 400 error
        snprintf(out_header, BUF_SIZE, "HTTP/1.1 %s\r\n\r\n", CODE_400);
        write(new_sock, out_header, strlen(out_header));
        if(log_flag != 0){
        Log_Fail_Write(request, filename, 400);
        }
    }
    else if(strlen(filename) != 27){
        snprintf(out_header, BUF_SIZE, "HTTP/1.1 %s\r\n\r\n", CODE_400);
        write(new_sock, out_header, strlen(out_header));
        if(log_flag != 0){
        Log_Fail_Write(request, filename, 400);
        }
    }
    else{
       protocol = strtok(NULL, delimit);
       if(strcmp("PUT", request) == 0){
           //printf("In GET\n");
         token = strtok(NULL, delimit);

         while(strcmp(token,"Content-Length:") != 0 ){
           token = strtok(NULL, delimit);
         }
         file_length = atoi(strtok(NULL,delimit));
         int fd = open(filename, O_RDONLY | O_WRONLY | O_APPEND | O_CREAT | O_TRUNC);
         //fchmod(fd, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
         snprintf(status, sizeof(CODE_201), CODE_201);
         snprintf(out_header, BUF_SIZE, "HTTP/1.1 %s\r\n\r\n",status); 

         snprintf(status, sizeof(CODE_100), CODE_100);
         snprintf(out_header, BUF_SIZE, "HTTP/1.1 %s\r\n\r\n",status); 
         write(new_sock, out_header, strlen(out_header));

        if(log_flag != 0){
            sprintf(log_buff, "PUT %s length %d",filename, file_length);
            write(log_fd, log_buff, strlen(log_buff));
        }
        int log_counter = 0;
         if(no_length == 0){
            

            
           for(int x = 0; x < file_length; x++){
            if(log_counter % 20 == 0 && log_flag != 0){
                sprintf(log_buff, "\n%08d", log_counter);
                write(log_fd, log_buff, strlen(log_buff));
                //printf("in log\n");
            }
            read(new_sock, data, 1);
            write(fd, data, 1);
            
            if(log_flag != 0){
                sprintf(log_buff, " %02x", (uint)*data);
                write(log_fd, log_buff, strlen(log_buff));
            }
            log_counter++;

           }
           if(log_flag != 0){
           sprintf(log_buff, "\n========\n");
            write(log_fd, log_buff, strlen(log_buff));
           }
         }
        else{
            while(read(new_sock, data, 1) != 0){
                write(fd, data, 1);
            if(log_counter % 20 == 0 && log_flag != 0){
                sprintf(log_buff, "\n%08d", log_counter);
                write(log_fd, log_buff, strlen(log_buff));
                //printf("in log\n");
            }
            if(log_flag != 0){
                sprintf(log_buff, " %02x", (uint)*data);
                write(log_fd, log_buff, strlen(log_buff));
            }
            }
        }
        fchmod(fd, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
        close(fd);
        snprintf(status, sizeof(CODE_201), CODE_201);
        snprintf(out_header, BUF_SIZE, "HTTP/1.1 %s\r\nContent-length:0\r\n\r\n",status); 
        write(new_sock, out_header, strlen(out_header));
    }
    else if(strcmp("GET", request) == 0){
       // printf("In GET\n");
        int fd = open(filename, O_RDONLY);
        if(fd < 0){//if fd is less than 0 an error has occured
            if(errno == EACCES){//checks if we can access the file
                snprintf(status, sizeof(CODE_403), CODE_403);
                if(log_flag != 0){
                Log_Fail_Write(request, filename, 403);
                }
            }
            else{//otherwise, not found 404
                snprintf(status, sizeof(CODE_404), CODE_404);
                if(log_flag != 0){
                Log_Fail_Write(request, filename, 404);
                }
            }
            snprintf(out_header, BUF_SIZE, "HTTP/1.1 %s\r\nContent-Length:\r\n\r\n", status);
            write(new_sock, out_header, strlen(out_header));
        }
        else{//at this point, no error in retreiving the file, thus 200
          snprintf(status,sizeof(CODE_200), CODE_200);
          memset(data, 0, BUF_SIZE);//reset data buffer
          //read(fd, data, BUF_SIZE);
          fstat(fd, &for_size);
          file_length = for_size.st_size;

          snprintf(out_header, BUF_SIZE, "HTTP/1.1 %s\r\nContent-Length:%d\r\n\r\n", status, file_length);
          write(new_sock, out_header, strlen(out_header));//write header before data
            if(log_flag != 0){
            sprintf(log_buff, "GET %s length %d",filename, 0);
            write(log_fd, log_buff, strlen(log_buff));
            sprintf(log_buff, "\n========\n");
            write(log_fd, log_buff, strlen(log_buff));

        }
        for(int x = 0; x < file_length; x++){//one byte at a time until file length
            read(fd, data, 1);
            write(new_sock, data, 1);
          }
          close(fd);

        }
    }
    else{//If request is not GET or PUT, status code 500
        sscanf(status, CODE_500);
        sscanf(out_header,"HTTP/1.1 %s\r\n", status);
        write(new_sock, out_header, strlen(out_header));
    }
    //printf("All done\n%s \nRequest:%s\nFilename:%s\nProtocol:%s\nFile Length:%d\n", buffer,request,filename,protocol,file_length);
    //send(new_sock,"HTTP/1.1 200 OK\r\n", 8, 0);
    }
    close(new_sock);
    
    


        pthread_mutex_unlock(&mutex_file);

        pthread_mutex_lock(&mutex_log);
        //Write to log file here



        pthread_mutex_unlock(&mutex_log);



        //printf("end of thread loop\n");

    }
    
}




