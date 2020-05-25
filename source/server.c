#include "common.h"

struct FineVillageAccount
{
	int size;
	char memberName[50][80];
	int requestMoney[10];
	int totalMoney;
}fineAccount;



enum CASE
{
	NAME, MONEY
};

#define CERTFILE "server.pem"
SSL_CTX *setup_server_ctx(void)
{
	SSL_CTX	*ctx;

	ctx = SSL_CTX_new(SSLv23_method());
	if(SSL_CTX_use_certificate_chain_file(ctx,CERTFILE) != 1)
		int_error("Error loading certificate from file");
	if(SSL_CTX_use_PrivateKey_file(ctx, CERTFILE, SSL_FILETYPE_PEM) != 1)
		int_error("Error loading private key from file");
	return ctx;
}

int do_server_loop(SSL *ssl)
{
	int err, nread;
	char buf[80];

	char name[80];
	int num = 0;
	int temp;
	int count = 0;

	int flag = NAME;

	do
	{
		for(nread=0; nread<sizeof(buf); nread +=err)
		{
			err = SSL_read(ssl, buf + nread, sizeof(buf) - nread);
			if(err <=0)
				break;
		}

		if (buf[0] == '-') {
			fprintf(stderr, "LOG OUT\n");
			return 0;
		}

		fprintf(stdout, "----------------------------------\n");
		if (flag == NAME) {

			strcpy(name, buf);
			flag = MONEY;

			if (name[0] == '\0') {
				fprintf(stdout, "NONAME is connecting...\n");
			}
			else {
				fprintf(stdout, "%s is connecting...\n", name);
			}
		}

		else if (flag == MONEY) {

			buf[strlen(buf) - 1] = '\0';

			num = 0;

			for (int i = 0; i < strlen(buf); i++) {
				temp = buf[i] - '0';

				if (temp >= 0 && temp <= 9) {
					num += temp;
					count++;
				}

				if (i == strlen(buf) - 1) {

				}
				else {
					num = num * 10;
				}

			}
			if (count == strlen(buf)) {
				fprintf(stdout, "%s charge money : %dwon\n", name, num);
				strcpy(fineAccount.memberName[fineAccount.size], name);
				fineAccount.requestMoney[fineAccount.size] = num;
				fineAccount.size++;
				fineAccount.totalMoney += num;
				
				;
				fprintf(stdout, "----------<REQUEST LIST>----------\n");
				for (int i = 0; i < fineAccount.size; i++) {
					fprintf(stdout, "%s \t : %dwon\n", fineAccount.memberName[i], fineAccount.requestMoney[i]);
				}
				fprintf(stdout, "----------------------------------\n");
				fprintf(stdout, "total request : %dwon\n", fineAccount.totalMoney);
			}
			else {
				fprintf(stdout, "this is wrong insert.\n");
			}
			count = 0;

		}
		fprintf(stdout, "----------------------------------\n");
	}
	while(err>0);
	return (SSL_get_shutdown(ssl) & SSL_RECEIVED_SHUTDOWN) ? 1:0;
}

void THREAD_CC server_thread(void *arg)
{
	SSL *ssl = (SSL *)arg;

#ifndef WIN32
	pthread_detach(pthread_self());
#endif
	if (SSL_accept(ssl) <= 0 )
		int_error("Error accepting SSL connection");
	fprintf(stderr, "SSL Connection opened\n");
	if(do_server_loop(ssl))
		SSL_shutdown(ssl);
	else
		SSL_clear(ssl);
	fprintf(stderr, "SSL Connection closed\n");
	SSL_free(ssl);

	ERR_remove_state(0);

#ifdef WIN32
	_endthread();
#endif
}

int main(int argc, char *argv[])
{
	BIO	*acc, *client;
	SSL	*ssl;
	SSL_CTX	*ctx;
	THREAD_TYPE	tid;

	init_OpenSSL();
	seed_prng();

	ctx=setup_server_ctx();

	acc=BIO_new_accept(PORT);
	if(!acc)
		int_error("Error creating server socket");
	if(BIO_do_accept(acc) <= 0)
		int_error("Error binding server socket");

	fprintf(stderr, "This is Fine Village MASTER account!\n");
	fineAccount.size = 0;
	fineAccount.totalMoney = 0;

	for(;;)
	{
		if(BIO_do_accept(acc) <= 0)
			int_error("Error accepting connection");

		client = BIO_pop(acc);
		if(!(ssl = SSL_new(ctx)))
			int_error("Error creating SSL context");

		SSL_set_bio(ssl, client, client);
		THREAD_CREATE(tid, (void *)server_thread, ssl);
	}

	SSL_CTX_free(ctx);
	BIO_free(acc);
	return 0;
}

