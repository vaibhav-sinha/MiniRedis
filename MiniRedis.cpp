#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <strings.h>
#include <pthread.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <map>
#include <string>
#include <iostream>
#include "nargv.h"
#include "bitv.h"
#include "zclass.h"
#include "timer.h"
#include "strclass.h"
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>
#include <stdexcept>
#include <csignal>
#include <fstream>

std::map<std::string,strclass> strmap;
std::map<std::string,timer> strexmap;
std::map<std::string,bitv> bitmap;
std::map<std::string,zclass> zmap;
std::string db_file;

class sid {
	public:
		int oid,nid;
};

void dump() {
	std::map<std::string,strclass>::iterator strit;
	std::map<std::string,zclass>::iterator zit;
	std::map<std::string,double>::iterator oit;
	std::ofstream f;
	f.open(db_file);
	for(strit = strmap.begin(); strit != strmap.end(); strit++) {
		f << "STRING\n";
		f << strit->first << "\n";
		if(strexmap.find(strit->first) != strexmap.end()) {
			f << strexmap.find(strit->first)->second.time << "\n";
		}
		else {
			f << "-1\n";
		}
		f << strit->second.str.c_str() << "\n";
		//f << strit->second.v.size() << "\n";
		//for(int k=0; k<strit->second.v.size(); k++) {
		//	f << strit->second.v[k].to_string() << "\n";
		//}
		f << "END\n";
	}
	for(zit = zmap.begin(); zit != zmap.end(); zit++) {
		f << "ZMAP\n";
		f << zit->first << "\n";
		f << zit->second.card() << "\n";
		for(oit = zit->second.orig.begin(); oit != zit->second.orig.end(); oit++) {
			f << oit->first << "\n";
			f << oit->second << "\n";
		}
		f << "END\n";
	}
	f.close();
}

void read_db() {
	std::ifstream f;
	std::string line;
	std::string key;
	std::string member;
	std::string temp;
	std::string str;
	std::string inp;
	strclass s;
	zclass z;
	int time;
	int num;
	double score;
	int cnt;

	f.open(db_file);
	while(getline(f,line)) {
		if(line == "STRING") {
			getline(f,key);
			getline(f,temp);
			time = atoi(temp.c_str());
			getline(f,str);
			//getline(f,temp);
			//num = atoi(temp);
			//s.set(str);
			strmap[key] = strclass(str);
			getline(f,temp);
			if(temp.compare("END") != 0) std::cout<<"Error in DB file for key= \n" << key;
		}
		else if(line == "ZMAP") {
			std::cout<<"Got zmap\n";
			getline(f,key);
			getline(f,temp);
			num = atoi(temp.c_str());
			zmap[key] = z;
			for(int k=0; k<num; k++) {
				getline(f,member);
				getline(f,temp);
				score = atof(temp.c_str());
				zmap[key].add(member,score);
			}
			getline(f,temp);
			if(temp.compare("END") != 0) std::cout<<"Error in DB file for key= \n" << key;
		}
	}
	f.close();
}

void cleanup (int param)
{
	std::cout<<"Dumping database to file\n";
	dump();
	printf ("Terminating program...\n");
	exit(1);
}

void *expiry_callback(info* i) {
	while(*(i->t)>0) {
		usleep(1000);
		*(i->t) = *(i->t) - 1;
	}
	if(strmap.find(i->key)!= strmap.end()) {
		strmap.erase(i->key);
	}
}

void send_response (int sock, char* buf, int size)
{
	int n;
	std::string b = std::string(buf);
	size = b.length();
	n = write(sock,buf,size);
	if (n < 0) 
	{
		perror("ERROR writing to socket");
		exit(1);
	}
}

void *doprocessing (sid* socks)
{
	int n,arg;
	char buffer[256];
	char resp[256];
	bool p;
	int sock = socks->nid;
	std::string skey,svalue;
	std::string gkey,gvalue;
	bitv temp_bitv;
	strclass temp_strclass;
	//close(socks->oid);

	while(1) {
		bzero(buffer,256);
		n = read(sock,buffer,255);
		if (n < 0)
		{
			perror("ERROR reading from socket");
			exit(1);
		}

		buffer[std::string(buffer).length()-2] = '\n';
		NARGV *nargv = nargv_parse(buffer);
		if (nargv->error_code) {
			printf("\nnargv parse error: %i: %s: at input column %i\n", nargv->error_code, nargv->error_message, nargv->error_index);
		} else {
			if(strcasecmp(nargv->argv[0],"SET") == 0) {
				if(nargv->argc != 3) {
					sprintf(resp,"error:wrong number of args (got %d for 2)\n",nargv->argc-1);
					send_response(sock,resp,40);
				}
				else {
					skey = std::string(nargv->argv[1]);
					svalue = std::string(nargv->argv[2]);
					strmap[skey] = strclass::strclass(svalue);
					sprintf(resp,"OK\n");
					send_response(sock,resp,3);
				}
			}
			else if(strcasecmp(nargv->argv[0],"SETNX") == 0) {
				if(nargv->argc != 3) {
					sprintf(resp,"error:wrong number of args (got %d for 2)\n",nargv->argc-1);
					send_response(sock,resp,40);
				}
				else {
					skey = std::string(nargv->argv[1]);
					svalue = std::string(nargv->argv[2]);
					strmap.insert(std::make_pair(skey,strclass::strclass(svalue)));
					sprintf(resp,"OK\n");
					send_response(sock,resp,3);
				}
			}
			else if(strcasecmp(nargv->argv[0],"SETXX") == 0) {
				if(nargv->argc != 3) {
					sprintf(resp,"error:wrong number of args (got %d for 2)\n",nargv->argc-1);
					send_response(sock,resp,40);
				}
				else {
					skey = std::string(nargv->argv[1]);
					svalue = std::string(nargv->argv[2]);
					if(strmap.find(skey) != strmap.end()) {
						strmap[skey] = strclass::strclass(svalue);
						sprintf(resp,"OK\n");
						send_response(sock,resp,3);
					}
				}
			}
			else if(strcasecmp(nargv->argv[0],"SETEX") == 0) {
				if(nargv->argc != 4) {
					sprintf(resp,"error:wrong number of args (got %d for 3)\n",nargv->argc-1);
					send_response(sock,resp,40);
				}
				else {
					skey = std::string(nargv->argv[1]);
					svalue = std::string(nargv->argv[3]);
					timer ms;
					strmap[skey] = strclass::strclass(svalue);
					strexmap[skey] = ms;
					strexmap[skey].run(skey,atoi(nargv->argv[2])*1000);
					sprintf(resp,"OK\n");
					send_response(sock,resp,3);
				}
			}
			else if(strcasecmp(nargv->argv[0],"PSETEX") == 0) {
				if(nargv->argc != 4) {
					sprintf(resp,"error:wrong number of args (got %d for 3)\n",nargv->argc-1);
					send_response(sock,resp,40);
				}
				else {
					skey = std::string(nargv->argv[1]);
					svalue = std::string(nargv->argv[3]);
					timer ms;
					strmap[skey] = strclass::strclass(svalue);
					strexmap[skey] = ms;
					strexmap[skey].run(skey,atoi(nargv->argv[2]));
					sprintf(resp,"OK\n");
					send_response(sock,resp,3);
				}
			}
			else if(strcasecmp(nargv->argv[0],"GET") == 0) {
				if(nargv->argc != 2) {
					sprintf(resp,"error:wrong number of args (got %d for 1)\n",nargv->argc-1);
					send_response(sock,resp,40);
				}
				else {
					gkey = std::string(nargv->argv[1]);
					if(strmap.find(gkey) == strmap.end()) {
						sprintf(resp,"nil\n");
						send_response(sock,resp,4);
					}
					else {
						sprintf(resp,"%s\n",strmap[nargv->argv[1]].str.c_str());
						send_response(sock,resp,strmap[nargv->argv[1]].str.length()+1);
					}
				}
			}
			else if(strcasecmp(nargv->argv[0],"SETBIT") == 0) {
				if(nargv->argc != 4) {
					sprintf(resp,"error:wrong number of args (got %d for 3)\n",nargv->argc-1);
					send_response(sock,resp,40);
				}
				else {
					skey = std::string(nargv->argv[1]);
					strmap.insert(std::make_pair(skey,temp_strclass)); 
					p = strmap[skey].setbit(atoi(nargv->argv[2]),atoi(nargv->argv[3]));
					sprintf(resp,"%d\n",p);
					send_response(sock,resp,2);
				}
			}
			else if(strcasecmp(nargv->argv[0],"GETBIT") == 0) {
				if(nargv->argc != 3) {
					sprintf(resp,"error:wrong number of args (got %d for 2)\n",nargv->argc-1);
					send_response(sock,resp,40);
				}
				else {
					gkey = std::string(nargv->argv[1]);
					p = strmap[skey].getbit(atoi(nargv->argv[2]));
					sprintf(resp,"%d\n",p);
					send_response(sock,resp,2);
				}
			}
			else if(strcasecmp(nargv->argv[0],"OSETBIT") == 0) {
				if(nargv->argc != 4) {
					sprintf(resp,"error:wrong number of args (got %d for 3)\n",nargv->argc-1);
					send_response(sock,resp,40);
				}
				else {
					skey = std::string(nargv->argv[1]);
					bitmap.insert(std::make_pair(skey,temp_bitv)); 
					if(bitmap[skey].data.size() > atoi(nargv->argv[2])) {
						sprintf(resp,"%d\n",bitmap[skey].data.at(atoi(nargv->argv[2])));
					}
					else {
						sprintf(resp,"%d\n",0);
					}
					bitmap[skey].set(atoi(nargv->argv[2]),atoi(nargv->argv[3]));
					send_response(sock,resp,2);
				}
			}
			else if(strcasecmp(nargv->argv[0],"OGETBIT") == 0) {
				if(nargv->argc != 3) {
					sprintf(resp,"error:wrong number of args (got %d for 2)\n",nargv->argc-1);
					send_response(sock,resp,40);
				}
				else {
					gkey = std::string(nargv->argv[1]);
					if(bitmap[gkey].data.size() > atoi(nargv->argv[2])) {
						sprintf(resp,"%d\n",bitmap[gkey].data.at(atoi(nargv->argv[2])));
					}
					else {
						sprintf(resp,"%d\n",0);
					}
					send_response(sock,resp,2);
				}
			}
			else if(strcasecmp(nargv->argv[0],"ZADD") == 0) {
				if(nargv->argc != 4) {
					sprintf(resp,"error:wrong number of args (got %d for 3)\n",nargv->argc-1);
					send_response(sock,resp,40);
				}
				else {
					skey = std::string(nargv->argv[1]);
					if(zmap.find(skey) == zmap.end()) {
						if(strmap.find(skey) == strmap.end()) {
							zclass temp_z;
							temp_z.add(std::string(nargv->argv[3]),atof(nargv->argv[2]));
							zmap.insert(std::make_pair(skey,temp_z));
						}
						else {
							sprintf(resp,"error:WRONGTYPE\n");
							send_response(sock,resp,40);
						}
					}
					else {
						zmap.find(skey)->second.add(std::string(nargv->argv[3]),atof(nargv->argv[2]));
					}
				}
			}
			else if(strcasecmp(nargv->argv[0],"ZCARD") == 0) {
				if(nargv->argc != 2) {
					sprintf(resp,"error:wrong number of args (got %d for 1)\n",nargv->argc-1);
					send_response(sock,resp,40);
				}
				else {
					skey = std::string(nargv->argv[1]);
					if(zmap.find(skey) == zmap.end()) {
						std::cout<<"Key not found\n";
						sprintf(resp,"%d\n",0);
						send_response(sock,resp,2);
					}
					else {
						sprintf(resp,"%d\n",zmap.find(skey)->second.card());
						send_response(sock,resp,9);
					}
				}
			}
			else if(strcasecmp(nargv->argv[0],"ZCOUNT") == 0) {
				if(nargv->argc != 4) {
					sprintf(resp,"error:wrong number of args (got %d for 3)\n",nargv->argc-1);
					send_response(sock,resp,40);
				}
				else {
					skey = std::string(nargv->argv[1]);
					if(zmap.find(skey) == zmap.end()) {
						sprintf(resp,"%d\n",0);
						send_response(sock,resp,2);
					}
					else {
						sprintf(resp,"%d\n",zmap.find(skey)->second.count(atoi(nargv->argv[2]),atoi(nargv->argv[3])));
						send_response(sock,resp,9);
					}
				}
			}
			else if(strcasecmp(nargv->argv[0],"ZRANGE") == 0) {
				if(nargv->argc != 4) {
					sprintf(resp,"error:wrong number of args (got %d for 3)\n",nargv->argc-1);
					send_response(sock,resp,40);
				}
				else {
					skey = std::string(nargv->argv[1]);
					if(zmap.find(skey) == zmap.end()) {
						sprintf(resp,"%d\n",0);
						send_response(sock,resp,2);
					}
					else {
						std::string p = zmap.find(skey)->second.range(atoi(nargv->argv[2]),atoi(nargv->argv[3])).c_str();
						sprintf(resp,"%s\n",p.c_str());
						send_response(sock,resp,256);
					}
				}
			}
			else if(strcasecmp(nargv->argv[0],"SAVE") == 0) {
				if(nargv->argc != 1) {
					sprintf(resp,"error:wrong number of args (got %d for 2)\n",nargv->argc-1);
					send_response(sock,resp,40);
				}
				else {
					dump();
					sprintf(resp,"OK\n");
					send_response(sock,resp,3);
				}
			}
			else if(strcasecmp(nargv->argv[0],"END") == 0) {
				close(sock);
				break;
			}
			else {
				printf("Got %s. Invalid command\n",nargv->argv[0]);
			}
		}
		nargv_free(nargv);
	}
}

int main( int argc, char *argv[] )
{
	int sockfd, newsockfd, portno, clilen;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	int  n;
	pid_t pid;
	pthread_t thread;
	sid s;

	if(argc>1) {
		db_file = argv[1];
		read_db();
	}
	else {
		db_file = "db_dump";
	}

	//Signal handling
	void (*prev_fn)(int);
	prev_fn = signal (SIGINT,cleanup);
	if (prev_fn==SIG_IGN) signal (SIGINT,SIG_IGN);

	/* First call to socket() function */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
	{
		perror("ERROR opening socket");
		exit(1);
	}
	/* Initialize socket structure */
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = 15000;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	/* Now bind the host address using bind() call.*/
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
				sizeof(serv_addr)) < 0)
	{
		perror("ERROR on binding");
		exit(1);
	}
	/* Now start listening for the clients, here 
	 * process will go in sleep mode and will wait 
	 * for the incoming connection
	 */
	listen(sockfd,5);
	clilen = sizeof(cli_addr);
	while (1) 
	{
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t*) &clilen);
		if (newsockfd < 0)
		{
			perror("ERROR on accept");
			exit(1);
		}
		/* Create child thread */
		s.oid = sockfd;
		s.nid = newsockfd;
		pthread_create(&thread, NULL, doprocessing, &s);
	} /* end of while */
	close(sockfd);
}
