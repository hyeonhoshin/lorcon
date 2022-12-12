/*
	beacon_flood_raw.c 
	by brad.antoniewicz@foundstone.com	

	simple IEEE 802.11 beacon flooder using LORCON2's 
	raw sending capabilities

*/
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <lorcon2/lorcon.h> // For LORCON 





#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <sys/time.h>
#include <arpa/inet.h>

#include <lorcon2/lorcon.h>
#include <lorcon2/lorcon_forge.h>
#include <lorcon2/lorcon_packasm.h>




void usage(char *argv[]) {
	printf("\t-i <int> \tInterface\n");
	printf("\t-c <channel>\tChannel\n");
	printf("\t-l <int> \tlength\n");
	printf("\nExample:\n");
	printf("\t%s -i wlan0 -c 11HT20 -l 900\n\n",argv[0]);
}
int main(int argc, char *argv[]) {

	printf("============= Tanuki Injector v0.0 ==============\n");

	char *interface = NULL, *ssid = NULL;
	int c;
	unsigned int count=0;
	int length=5000;
	lorcon_driver_t *drvlist, *driver; // Needed to set up interface/context
	lorcon_t *context; // LORCON context

 	//int interval = 5;

	int channel, ch_flags;
   int num;
    char current;
    int rr;
   FILE *fptr;

	/* 
		This handles all of the command line arguments
	*/
	
	while ((c = getopt(argc, argv, "hi:c:l:")) != EOF) {
		switch (c) {
			case 'i': 
				interface = strdup(optarg);
				break;
			case 'c':
				if (lorcon_parse_ht_channel(optarg, &channel, &ch_flags) == 0) {
			    		printf("ERROR: Unable to parse channel\n");
			    		return -1;
				}
				break;

			case 'l':
				//length = atoi(optarg);
				sscanf(optarg, "%d", &length);
				break;
			case 'h':
				usage(argv);
				break;
			default:
				usage(argv);
				break;
			}
	}

	if ( interface == NULL  ) { 
		printf ("ERROR: Interface not set (see -h for more info)\n");
		return -1;
	}

	printf("[+] Using interface %s\n Inject 20MHz packets.\n",interface);
				printf("\n length is %d\n",length);
	uint8_t packet[5000] = {0x00};
	char fname_full[200] = {'\0'};


	if ( (driver = lorcon_auto_driver(interface)) == NULL) {
		printf("[!] Could not determine the driver for %s\n",interface);
		return -1;
	} else {
		printf("[+]\t Driver: %s\n",driver->name);
	}

	// Create LORCON context
        if ((context = lorcon_create(interface, driver)) == NULL) {
                printf("[!]\t Failed to create context");
               	return -1; 
        }

	// Create Monitor Mode Interface
	if (lorcon_open_injmon(context) < 0) {
		printf("[!]\t Could not create Monitor Mode interface!\n");
		return -1;
	} else {
		printf("[+]\t Monitor Mode VAP: %s\n",lorcon_get_vap(context));
		lorcon_free_driver_list(driver);
	}



	lorcon_set_ht_channel(context, channel, ch_flags);

	printf("[+]\t Using channel: %d\n\n",channel);

	printf("[+]\t length is: %d\n\n",length);

	char seq[2]="0";

	int file_seq=0;
	int rrr=0;

///////   send out 6*127 copies of "Input_Bytes.txt"
	for (rrr=0;rrr<6;rrr++){ // Repeat 6 times.

	    for (file_seq=0;file_seq<127;file_seq++){

			sprintf(fname_full, "./gen_frag_packet/gen_pkt_{%d}.txt",file_seq); // The number indicates scrambling seed.

/////////      read file Input_Bytes.txt
			if ((fptr = fopen(fname_full,"r")) == NULL){
				printf("Error! opening file\n");

				// Program exits if the file pointer returns NULL.
				exit(1);
			}

			for (num=0;num<length;num++){
				fscanf(fptr,"%d\n", &rr);
				packet[num]=(uint8_t)rr;
			}
			//printf("[+]\t length is: %d\n\n",length);
			if ( lorcon_send_bytes(context, sizeof(char)*length, packet) < 0 ) 
				return -1;

			// Wait interval before next beacon
				usleep(1000);

			// Print nice and pretty
			printf("\033[K\r");
			printf("[+] Sent %d frames, Hit CTRL + C to stop...", count+1);
			fflush(stdout);
			count++;
	    }
	}

    printf("\n");
	// Close the interface
	lorcon_close(context);

	// Free the LORCON Context
	lorcon_free(context);	
	
	return 0;
}

