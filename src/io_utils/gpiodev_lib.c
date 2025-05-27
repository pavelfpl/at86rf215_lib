/**
 * GPIO library for /dev/gpiodevX
 * Pavel Fiala @ 2024
 **/

#include <sys/ioctl.h>
#include <sys/poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <getopt.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#include "gpiodev_lib.h"

// Enable GPIO debug option
// #define GPIODEV_DEBUG

typedef struct gpio_event_t{
    char *dev_name;
    int offset;
    int thread_status;
    uint32_t event_flags;
    time_t wait_time;
    void (*gpio_event_isr_callback)(void *user_param, void *user_data);
    void *gpio_event_isr_param;
    void *gpio_event_isr_user_data;
}gpio_event_s;

// Static variables
int pipes[2];
static pthread_t event_thread;
static gpio_event_s event_struct;
// Static function
void *gpio_poll_wait_thread(void *ptr);

/* GPIO get info */
void gpio_list(const char *dev_name){
    
    int fd = 0, ret=0;
    
    struct gpiochip_info info = {0};
    struct gpioline_info line_info = {0};
    
    fd = open(dev_name, O_RDONLY | O_NONBLOCK);
    
    if(fd < 0){
#ifdef GPIODEV_DEBUG         
       printf("Unabled to open %s: %s", dev_name, strerror(errno));
#endif
       return;
    }
    
    // GPIO_GET_CHIPINFO_IOCTL
    ret = ioctl(fd, GPIO_GET_CHIPINFO_IOCTL, &info);
    
    if(ret == -1){
#ifdef GPIODEV_DEBUG        
       printf("Unable to get chip info from ioctl: %s", strerror(errno));
#endif
       close(fd);
       return;
    }
    
    // Show gpio info ...
    printf("Chip name: %s\n", info.name);
    printf("Chip label: %s\n", info.label);
    printf("Number of lines: %d\n", info.lines);
 
    // Iterate over gpio lines ...
    for (int i = 0; i < info.lines; i++){
        line_info.line_offset = i;
        
        // GPIO_GET_LINEINFO_IOCTL
        ret = ioctl(fd, GPIO_GET_LINEINFO_IOCTL, &line_info);
        
        if (ret == -1){
            printf("Unable to get line info from offset %d: %s", i, strerror(errno));
        }else{
            printf("offset: %d, name: %s, consumer: %s. Flags:\t[%s]\t[%s]\t[%s]\t[%s]\t[%s]\n",
                   i,
                   line_info.name,
                   line_info.consumer,
                   (line_info.flags & GPIOLINE_FLAG_IS_OUT) ? "OUTPUT" : "INPUT",
                   (line_info.flags & GPIOLINE_FLAG_ACTIVE_LOW) ? "ACTIVE_LOW" : "ACTIVE_HIGHT",
                   (line_info.flags & GPIOLINE_FLAG_OPEN_DRAIN) ? "OPEN_DRAIN" : "...",
                   (line_info.flags & GPIOLINE_FLAG_OPEN_SOURCE) ? "OPENSOURCE" : "...",
                   (line_info.flags & GPIOLINE_FLAG_KERNEL) ? "KERNEL" : "");
        }
    }
    
    close(fd);
}

/* GPIO write single GPIO */
void gpio_write_single(const char *dev_name, int offset, uint8_t value){
    
    int fd=0, ret=0;
    
    struct gpiohandle_request rq = {0};
    struct gpiohandle_data data = {0};

#ifdef GPIODEV_DEBUG
    printf("Write value %d to GPIO at offset %d (OUTPUT mode) on chip %s\n", value, offset, dev_name);
#endif 
    
    fd = open(dev_name, O_RDONLY | O_NONBLOCK);
    
    if(fd < 0){
#ifdef GPIODEV_DEBUG        
       printf("Unabled to open %s: %s", dev_name, strerror(errno));
#endif       
       return;
    }
    
    rq.lineoffsets[0] = offset;
    // Set GPIO as output ...
    rq.flags = GPIOHANDLE_REQUEST_OUTPUT;
    rq.lines = 1;
    // Request a GPIO line or lines from the kernel ...
    ret = ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &rq);
    // Close fd ...
    close(fd);
    
    if(ret == -1){
#ifdef GPIODEV_DEBUG         
       printf("Unable to line handle from ioctl : %s", strerror(errno));
#endif          
       return;
    }
    
    data.values[0] = value;
    ret = ioctl(rq.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);
    
    if (ret == -1){
#ifdef GPIODEV_DEBUG        
        printf("Unable to set line value using ioctl : %s", strerror(errno));
#endif        
        return;
    }

    // Close rq.fd ...
    close(rq.fd);
}

/* GPIO read single GPIO */
int gpio_read_single(const char *dev_name, int offset, uint8_t *value){
    
    int fd = 0, ret = 0;
    
    struct gpiohandle_request rq = {0};
    struct gpiohandle_data data = {0};
    
    fd = open(dev_name, O_RDONLY | O_NONBLOCK);
    
    if(fd < 0){
#ifdef GPIODEV_DEBUG            
        printf("Unabled to open %s: %s", dev_name, strerror(errno));
#endif        
        return -1;
    }
    
    rq.lineoffsets[0] = offset;
    // Set GPIO as input ...
    rq.flags = GPIOHANDLE_REQUEST_INPUT;
    rq.lines = 1;
    // Request a GPIO line or lines from the kernel ...
    ret = ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &rq);
    close(fd);
    
    if(ret == -1){
#ifdef GPIODEV_DEBUG        
       printf("Unable to get line handle from ioctl : %s", strerror(errno));
#endif          
       return -1;
    }
    
    ret = ioctl(rq.fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data);
    
    if(ret!=-1){
#ifdef GPIODEV_DEBUG         
       printf("Value of GPIO at offset %d (INPUT mode) on chip %s: %d\n", offset, dev_name, data.values[0]); 
#endif 
       *value = data.values[0];
    }else{
#ifdef GPIODEV_DEBUG         
       printf("Unable to get line value using ioctl : %s", strerror(errno));
#endif
      *value = 0;
      return -1;
    } 
    
    // Close rq.fd ...
    close(rq.fd);
    
    return 1;
 
}

/* GPIO poll single */
int gpio_poll_wait(const char *dev_name, int offset, int timeout, uint32_t event_flags){
    
    int fd = 0, ret = 0;
    
    struct gpioevent_request rq = {0};
    struct pollfd pfd = {0};
    
    fd = open(dev_name, O_RDONLY | O_NONBLOCK);
    
    if(fd < 0){
#ifdef GPIODEV_DEBUG           
        printf("Unabled to open %s: %s", dev_name, strerror(errno));
#endif        
        return -1;
    }
    
    rq.lineoffset = offset;
    rq.eventflags = event_flags; // GPIOEVENT_EVENT_RISING_EDGE
    ret = ioctl(fd, GPIO_GET_LINEEVENT_IOCTL, &rq);
    close(fd);
    
    if(ret == -1){
#ifdef GPIODEV_DEBUG         
        printf("Unable to get line event from ioctl : %s", strerror(errno));
#endif
        close(fd);
        return -1;
    }
    
    pfd.fd = rq.fd;
    pfd.events = POLLIN;
    ret = poll(&pfd, 1, timeout); // -1
    
    if(ret==-1){
#ifdef GPIODEV_DEBUG        
       printf("Error while polling event from GPIO: %s", strerror(errno));
#endif        
       return -1;
    } 
    
    if(pfd.revents & POLLIN){
#ifdef GPIODEV_DEBUG        
        printf("%s event on GPIO offset: %d, of %s\n", (rq.eventflags & GPIOEVENT_EVENT_RISING_EDGE) ? "Rising edge": "Falling edge", offset, dev_name);
#endif
       // Close rq.fd ...
       close(rq.fd);  
       // Return event type ... 
       return (rq.eventflags & GPIOEVENT_EVENT_RISING_EDGE) ? GPIOEVENT_EVENT_RISING_EDGE  : GPIOEVENT_EVENT_FALLING_EDGE;    
    }
    
    return -1;
}


void *gpio_poll_wait_thread(void *ptr){
    
    int fd = 0, ret = 0, res = 0;
    int gpio_event_enables = 0;
    
    struct gpioevent_request rq = {0};
    struct pollfd pfds[2] = {0};
    struct timespec remaining = {0};
    struct timespec request = {0};
    
    // Set to zero ...
    memset(&pfds, 0, sizeof(pfds));

    struct gpio_event_t *thread_ptr = (struct gpio_event_t *)ptr;
    
    pfds[0].fd = pipes[0];
    pfds[0].events = POLLIN;
    
    // Wait if thread_ptr->wait_time is higher then 0 ... 
    if(thread_ptr->wait_time !=0){
       request.tv_sec = 0;
       request.tv_nsec = thread_ptr->wait_time; 
    }
    
#ifdef GPIODEV_DEBUG    
    printf("Pthread will be executed ...\n");
#endif
    
    // While (1) loop ...
    while(1){
        if(!gpio_event_enables){
            fd = open(thread_ptr->dev_name, O_RDONLY | O_NONBLOCK);
    
            if(fd < 0){
#ifdef GPIODEV_DEBUG           
                printf("Unabled to open %s: %s", thread_ptr->dev_name, strerror(errno));
#endif        
                return NULL;
            } 
            // https://dri.freedesktop.org/docs/drm/userspace-api/gpio/gpio-handle-get-line-values-ioctl.html
            rq.handleflags = GPIOHANDLE_REQUEST_INPUT; /* !!! Do not use this option here: GPIOHANDLE_REQUEST_ACTIVE_LOW | */          
            rq.lineoffset = thread_ptr->offset;
            rq.eventflags = thread_ptr->event_flags; // GPIOEVENT_EVENT_RISING_EDGE or GPIOEVENT_EVENT_FALLING_EDGE
          
            ret = ioctl(fd, GPIO_GET_LINEEVENT_IOCTL, &rq);
            close(fd); fd = 0;
    
            if(ret == -1){
#ifdef GPIODEV_DEBUG         
               printf("Unable to get line event from ioctl : %s", strerror(errno));
#endif
               close(fd);
               return NULL;
            }  
            
            pfds[1].fd = rq.fd;
            pfds[1].events = POLLIN;
            
            gpio_event_enables = 1;
       }
        
       if(poll(pfds,2,-1)){ 
          // Check for errors ...
          if(errno == EINTR){
             continue;
          }
          
          // Terminate thread ...
          if(pfds[0].revents & POLLIN){
             read(pipes[0], &res, sizeof(res));
             if(res == 1) break;
          }
          
          if(pfds[1].revents & POLLIN){
#ifdef GPIODEV_DEBUG        
             printf("%s event on GPIO offset: %d, of %s\n", (rq.eventflags & GPIOEVENT_EVENT_RISING_EDGE) ? "Rising edge": "Falling edge", thread_ptr->offset, thread_ptr->dev_name);
#endif
             
             if(thread_ptr->gpio_event_isr_callback != NULL){
                // Check for rising edge event ... 
                if (rq.eventflags & GPIOEVENT_EVENT_RISING_EDGE){
                    // Execute callback function ... 
                    if(thread_ptr->gpio_event_isr_param != NULL){
                       *(int *)thread_ptr->gpio_event_isr_param = GPIOEVENT_EVENT_RISING_EDGE;   
                    }
                }
                
                // Check for falling edge ...
                if (rq.eventflags & GPIOEVENT_EVENT_FALLING_EDGE){
                    // Execute callback function ...
                    if(thread_ptr->gpio_event_isr_param != NULL){
                        *(int *)thread_ptr->gpio_event_isr_param = GPIOEVENT_EVENT_FALLING_EDGE;
                    }
                }
                
                // Close rq.fd ... 
                close(rq.fd);
                gpio_event_enables = 0; 
                
                // Wait if thread_ptr->wait_time is higher then 0 ... 
                if(thread_ptr->wait_time !=0){
                   nanosleep(&request, &remaining); 
                }
                
                // Call predefine callback ...  
                thread_ptr->gpio_event_isr_callback(thread_ptr->gpio_event_isr_param, thread_ptr->gpio_event_isr_user_data); // User data can be NULL pointer
                
             }

          }
       } 
    }
    
    // Closing thread session ...
#ifdef GPIODEV_DEBUG    
    printf("Pthread is about to terminate ...\n");
#endif
    
    // Close rq.fd ...
    if(gpio_event_enables) close(rq.fd);  
    
    return NULL;
    
}

/* gpio_poll_thread_start - start thread */
int gpio_poll_thread_start(char *dev_name, int offset, uint32_t event_flags,time_t wait_time, void (* p_callback)(void *user_param, void *user_data), void *p_param, void *p_user_data){
    
    // Pthread tutorial: https://www.cs.cmu.edu/afs/cs/academic/class/15492-f07/www/pthreads.html
    
    int  iret = -1, res = -1;
    
    res = pipe(pipes);
   
    if(res < 0){
       perror("pipe");
       return iret;
    }
        
    memset(&event_struct, 0, sizeof(event_struct));
    
    event_struct.dev_name = dev_name;
    event_struct.offset = offset;
    event_struct.event_flags = event_flags;
    event_struct.wait_time = wait_time;
    event_struct.gpio_event_isr_callback = p_callback;
    event_struct.gpio_event_isr_param = p_param;
    event_struct.gpio_event_isr_user_data = p_user_data;
    
    iret = pthread_create(&event_thread, NULL, gpio_poll_wait_thread, (void*)&event_struct);
   
    if(!iret){
#ifdef GPIODEV_DEBUG         
        printf("Starting thread ...\n");
#endif        
        event_struct.thread_status = 1; // Thread is running ...
    }else{
#ifdef GPIODEV_DEBUG          
        printf("Thread can not be started ...\n");
#endif
    }
    
    return iret;
    
}

/* gpio_poll_thread_stop - stop thread */
void gpio_poll_thread_stop(){
    
    int i = 1; // Stop running thread flag
    
    // Check thread_status -> 1 is running ...
    if(event_struct.thread_status){
#ifdef GPIODEV_DEBUG         
        printf("Stopping thread ...\n");
#endif                
       // Write pipe ... 
       write(pipes[1],&i,sizeof(i));  
       // pthread_join() function waits for the thread specified by thread to terminate
       pthread_join(event_thread, NULL);
       // Reset event_struct to 0 ...
       memset(&event_struct, 0, sizeof(event_struct));
       // Close pipe
       close(pipes[0]);
       close(pipes[1]);
    }
}


