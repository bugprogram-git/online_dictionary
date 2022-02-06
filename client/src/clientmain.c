#include <stdio.h>
#include <stdlib.h>
#include "client.h"
int  main(int argc,char **argv)
{
	int choose;
	if(argc !=3)
	{
		Usage(argv[0]);
	}
	else
	{
		int sockfd = conser(argv[1],argv[2]);
		if(sockfd == -1)
		{
			printf("connect remote server failed\n");
			return 0;
		}
		while(1)
		{
			mainmenu();
			scanf("%d",&choose);
			getchar();
			switch(choose)
			{
				case 1:
					do_register(sockfd);
					break;
				case 2:
					do_login(sockfd);
					break;
				case 3:
					quit_system(sockfd);
					exit(0);
				default:
					printf("invalid input:\n");
					break;
			}
		}
	}
	return 0;
}
