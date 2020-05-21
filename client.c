#include "common.h"

#define CERTFILE "client.pem"
SSL_CTX *setup_client_ctx(void)
{
	SSL_CTX *ctx;

	ctx = SSL_CTX_new(SSLv23_method());
	if(SSL_CTX_use_certificate_chain_file(ctx, CERTFILE) != 1)
		int_error("Error loading certificate from file");
	if(SSL_CTX_use_PrivateKey_file(ctx, CERTFILE, SSL_FILETYPE_PEM) != 1)
		int_error("Error loading private key from file");
	return ctx;
}

int do_client_loop(SSL *ssl)
{
	int err, nwritten;
	char buf[80];

	char clientName[80];
	char temp[80];

	while (130613) {
		fprintf(stderr, "what's your name? insert your name. >>");
		fgets(clientName, sizeof(clientName), stdin);

		clientName[strlen(clientName) - 1] = '\0';

		fprintf(stderr, "%s is your name. Right? (Y/N) >> ", clientName);
		fgets(temp, sizeof(temp), stdin);

		if (temp[0] == 'Y') {
			fprintf(stderr, "OK, %s is your name.\n", clientName);
			for (nwritten = 0; nwritten < sizeof(clientName); nwritten += err)
			{
				err = SSL_write(ssl, clientName + nwritten, sizeof(clientName) - nwritten);
				if (err <= 0)
					return 0;
			}
			break;
		}

	}

	for(;;)
	{
		fprintf(stderr, "How much money you need? >>");
		if(!fgets(buf, sizeof(buf), stdin))
			break;
		for(nwritten = 0; nwritten < sizeof(buf); nwritten += err)
		{
			err = SSL_write(ssl, buf+nwritten,sizeof(buf)-nwritten);
			if(err<=0)
				return 0;
		}

		if (buf[0] == '-') {
			fprintf(stderr, "LOG OUT\n");
			return 0;
		}
	}
	return 1;
}

int main(int argc, char *argv[])
{
	BIO	*conn;
	SSL	*ssl;
	SSL_CTX	*ctx;

	init_OpenSSL();
	seed_prng();

	ctx=setup_client_ctx();

	conn = BIO_new_connect(SERVER ":" PORT);
	if(!conn)
		int_error("Error creating connection BIO");
	if(BIO_do_connect(conn) <= 0)
		int_error("Error connecting to remote machine");
	if(!(ssl = SSL_new(ctx)))
		int_error("Error creating an SSL context");
	SSL_set_bio(ssl, conn, conn);
	if(SSL_connect(ssl) <= 0)
		int_error("Error connecting SSL object");

	fprintf(stderr, "SSL Connection opened\n");
	fprintf(stderr, "This is Fine Village account!\n");

	if(do_client_loop(ssl))
		SSL_shutdown(ssl);
	else
		SSL_clear(ssl);
	fprintf(stderr, "SSL Connection closed\n");

	SSL_free(ssl);
	SSL_CTX_free(ctx);
	return 0;
}

