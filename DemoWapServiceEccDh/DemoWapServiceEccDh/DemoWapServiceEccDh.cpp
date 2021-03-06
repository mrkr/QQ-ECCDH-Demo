
#include "pch.h"
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <openssl/ec.h>
#include <openssl/ecdh.h>
#include <openssl/md5.h>  
#include <openssl/ecdsa.h>
#include <openssl/objects.h>
#include <openssl/err.h>

#define MALLOC_SIZE 0x400u
#define PRI_KEY "3081db020101041853b24a1cece4f3acbde59d74c9e2d6062bdca43adaf18d0ca08185308182020101302406072a8648ce3d0101021900fffffffffffffffffffffffffffffffffffffffeffffee373006040100040103043104db4ff10ec057e9ae26b07d0280b7f4341da5d1b1eae06c7d9b2f2f6d9c5628a7844163d015be86344082aa88d95e2f9d021900fffffffffffffffffffffffe26f2fc170f69466a74defd8d020101a13403320004af8efd25576ed8ab550b5d65e0ad36916566708cbe5a3ad54e127ae42951d09212fc04a1ac463aff66d3dbf5ac6f46a1"
#define PUB_KEY "04af8efd25576ed8ab550b5d65e0ad36916566708cbe5a3ad54e127ae42951d09212fc04a1ac463aff66d3dbf5ac6f46a1"

using namespace std;


/*hex转bin*/
int String2Buffer(unsigned char* src, int srclen, unsigned char* dest) {
	int i = 0;
	if (srclen % 2 != 0) return 0;
	for (i = 0; i < srclen / 2; i++)
	{
		char tmp[3];
		tmp[0] = *(src + 2 * i);
		tmp[1] = *(src + 2 * i + 1);
		tmp[2] = 0;
		int out = 0;
		sscanf(tmp, "%x", &out);
		unsigned char ch = (unsigned char)out;
		*(dest + i) = ch;
	}
	return i;
}

/*bin转hex*/
int Buffer2String(unsigned char* src, int srclen, unsigned char* dest) {
	int i;
	for (i = 0; i < srclen; i++)
	{
		char tmp[3] = { 0 };
		sprintf(tmp, "%x", *(src + i));
		if (strlen(tmp) == 1) {
			strcat((char*)dest, "0");
			strncat((char*)dest, tmp, 1);
		}
		else if (strlen(tmp) == 2) {
			strncat((char*)dest, tmp, 2);
		}
		else {
			strcat((char*)dest, "00");
		}
	}
	return i * 2;
}

/*16进制展示数据*/
static void display(const char *tripstr, const char *src, const int src_size)
{
	int i = 0;
	if (tripstr != NULL) {
		printf("%s", tripstr);
	}
	unsigned char*tmp = (unsigned char*)malloc(MALLOC_SIZE);
	memset(tmp, 0, MALLOC_SIZE);
	Buffer2String((unsigned char*)src, src_size, tmp);
	cout << tmp << endl;
	free(tmp);
}

unsigned char * s_Sha_Key = (unsigned char*)malloc(MALLOC_SIZE); 
unsigned char * c_Sha_Key = (unsigned char*)malloc(MALLOC_SIZE); 
unsigned char * s_Pub_key = (unsigned char*)malloc(MALLOC_SIZE); 
unsigned char * c_Pub_key = (unsigned char*)malloc(MALLOC_SIZE); 
unsigned char * c_Pri_Key = (unsigned char*)malloc(MALLOC_SIZE); 

int main()
{
	EC_KEY *ec_key;
	const EC_GROUP *ec_group;
	const EC_POINT *ec_point;

	memset(s_Sha_Key, 0, MALLOC_SIZE); 
	memset(c_Sha_Key, 0, MALLOC_SIZE); 
	memset(s_Pub_key, 0, MALLOC_SIZE); 
	memset(c_Pub_key, 0, MALLOC_SIZE); 
	memset(c_Pri_Key, 0, MALLOC_SIZE);

	unsigned char buf[1024];
	unsigned char *pp;

	if ((ec_key = EC_KEY_new()) == NULL) {
		printf("Error：EC_KEY_new()\n");
		return 0;
	}

	/*选择一条椭圆曲线*/
	if ((ec_group = EC_GROUP_new_by_curve_name(NID_secp192k1)) == NULL) {
		printf("Error：EC_GROUP_new_by_curve_name()\n");
		EC_KEY_free(ec_key);
		return -1;
	}

	/*设置密钥参数*/
	int ret;
	ret = EC_KEY_set_group(ec_key, ec_group);
	if (ret != 1) {
		printf("Error：EC_KEY_set_group()\n");
		return -1;
	}

	String2Buffer((unsigned char*)PRI_KEY, 444, c_Pri_Key);
	String2Buffer((unsigned char*)PUB_KEY, 98, c_Pub_key);
	/* ---------------- 下面是导入 ---------------- */
	d2i_ECPrivateKey(&ec_key, (const unsigned char**)&c_Pri_Key, 222);
	o2i_ECPublicKey(&ec_key, (const unsigned char**)&c_Pub_key, 49);
	/* ---------------- 上面是导入 ---------------- */

	/*获取公钥*/
	if ((ec_point = EC_KEY_get0_public_key(ec_key)) == NULL) {
		printf("Error：EC_KEY_get0_public_key()\n");
		EC_KEY_free(ec_key);
		return -1;
	}

	/* ---------------- 下面是导出 ---------------- */
	///*导出私钥*/
	//pp = buf;
	//int len1 = i2d_ECPrivateKey(ec_key, &pp);//222
	//if (!len1) {
	//	printf("Error：i2d_ECPrivateKey()\n");
	//	EC_KEY_free(ec_key);
	//	return -1;
	//}
	//memcpy(c_Pri_Key, buf, len1);
	///*导出公钥*/
	//pp = buf;
	//int len2 = i2o_ECPublicKey(ec_key, &pp);//49
	//if (!len2) {
	//	printf("Error：i2o_ECPublicKey()\n");
	//	EC_KEY_free(ec_key);
	//	return-1;
	//}
	//memcpy(c_Pub_key, buf, len2);
	/* ---------------- 上面是导出 ---------------- */
	cout << "/*一次协商" << endl;
	char* pubkey = (char*)malloc(MALLOC_SIZE); memset(pubkey, 0, MALLOC_SIZE);
	cout << "c_Pub_Key ："; cin >> pubkey;
	int c_pub_len = String2Buffer((unsigned char*)pubkey, strlen(pubkey), c_Pub_key);
	/*密钥协商*/
	if (EC_POINT_oct2point(ec_group, (EC_POINT*)ec_point, c_Pub_key, c_pub_len, 0))
	{
		unsigned char *shared = (unsigned char *)malloc(MALLOC_SIZE); memset(shared, 0, MALLOC_SIZE);
		int c_sha_len = ECDH_compute_key(shared, 512, ec_point, ec_key, 0);
		MD5(shared, c_sha_len, c_Sha_Key);
		display("c_Sha_Key : ", (const char*)c_Sha_Key, MD5_DIGEST_LENGTH);
	}

	/*二次协商*/
	/*生成密钥对*/
	if (!EC_KEY_generate_key(ec_key)) {
		printf("Error：EC_KEY_generate_key()\n");
		EC_KEY_free(ec_key);
		return -1;
	}

	/*获取公钥*/
	if ((ec_point = EC_KEY_get0_public_key(ec_key)) == NULL) {
		printf("Error：EC_KEY_get0_public_key()\n");
		EC_KEY_free(ec_key);
		return -1;
	}

	/*导出公钥*/
	pp = buf;
	int len2 = i2o_ECPublicKey(ec_key, &pp);//49
	if (!len2) {
		printf("Error：i2o_ECPublicKey()\n");
		EC_KEY_free(ec_key);
		return-1;
	}
	memcpy(s_Pub_key, buf, len2);

	cout << "\n/*二次协商" << endl;
	display("s_Pub_key : ", (const char*)s_Pub_key, len2);

	/*二次协商*/
	if (EC_POINT_oct2point(ec_group, (EC_POINT*)ec_point, c_Pub_key, c_pub_len, 0))
	{
		unsigned char *shared = (unsigned char *)malloc(MALLOC_SIZE); memset(shared, 0, MALLOC_SIZE);
		int s_sha_len = ECDH_compute_key(shared, 512, ec_point, ec_key, 0);
		MD5(shared, s_sha_len, s_Sha_Key);
		display("s_Sha_Key : ", (const char*)s_Sha_Key, MD5_DIGEST_LENGTH);
	}

	EC_KEY_free(ec_key);

	getchar();
}