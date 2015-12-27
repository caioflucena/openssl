#include "php_p7s.h"

ZEND_BEGIN_ARG_INFO_EX(arginfo_openssl_pkcs_construct, 0, 0, 0)
    ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_openssl_pkcs_verify, 0, 0, 1)
    ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

PHP_METHOD(openssl_pkcs7, __construct) {
    int filenameLength = 0;
    char * filename = NULL;
    PKCS7 * p7s = NULL;
    //unsigned char ** signedContent = NULL;
    //zval * signedContentAttribute = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &filename, &filenameLength) == FAILURE) {
        return;
    }

    p7s = malloc(sizeof(PKCS7));
    if (getPkcs7FromFile(filename, p7s) == EXIT_FAILURE) {
        zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Could not read p7s file.", 0 TSRMLS_CC);
    }

    updatePropertySignatures(getThis(), p7s);
    updatePropertyIsDetached(getThis(), 1);

/*
    if (setSignedContent(p7s, signedContent) == EXIT_FAILURE) {
        zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Could not read signed content file.", 0 TSRMLS_CC);
    }
    //MAKE_STD_ZVAL(signedContentAttribute);
    if (NULL == *signedContent) {
        php_error(E_ERROR, "Oxi %s", *signedContent);

    }
    php_error(E_ERROR, "size %s", *signedContent);
    //ZVAL_STRING(signedContentAttribute, signedContent, 1);
    //zend_update_property(openssl_pkcs_p7s_ce, getThis(), "content", sizeof("content")-1, signedContentAttribute TSRMLS_CC);
*/
    free(p7s);
}

/**
 *
 * /
PHP_METHOD(openssl_pkcs7, getSignature) {
    zval * result;
           result = zend_read_property(openssl_pkcs_p7s_ce, getThis(), "signature", sizeof("signature")-1, 1 TSRMLS_CC);
    RETURN_ZVAL(result, 1, 0);
}

/**
 *
 * /
PHP_METHOD(openssl_pkcs7, getContent) {
    zval * result;
           result = zend_read_property(openssl_pkcs_p7s_ce, getThis(), "content", sizeof("content")-1, 1 TSRMLS_CC);
    RETURN_ZVAL(result, 1, 0);
}

/**
 *
 * /
PHP_METHOD(openssl_pkcs7, verify) {
    int filenameLength;
    char * filename;
    unsigned char * contentString;
    unsigned char * contentStringEncoded;
    FILE * file;
    zval * content;
    zval * result;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &filename, &filenameLength) == FAILURE) {
        return;
    }

    file = fopen(filename, "rb");
    if (NULL == file) {
        php_error(E_WARNING, "invalid file.");
        return;
    } else {
        int length;

        fseek(file, 0L, SEEK_END);
        length = ftell(file);
        fseek(file, 0L, SEEK_SET);

        contentString = (unsigned char *) malloc(length);
        fread(contentString, length, 1, file);
        bin_to_strhex(contentString, length, &contentStringEncoded);
        free(contentString);
    }
    fclose(file);

    content = zend_read_property(openssl_pkcs_p7s_ce, getThis(), "content", sizeof("content")-1, 1 TSRMLS_CC);
    if (NULL == content->value.str.val) {
        php_error(E_WARNING, "invalid content.");
        return;
    }

    MAKE_STD_ZVAL(result);
    if (strcmp(contentStringEncoded, content->value.str.val) == 0) {
        ZVAL_BOOL(result, 1);
    } else {
        ZVAL_BOOL(result, 0);
    }
    free(contentStringEncoded);

    RETURN_ZVAL(result, 1, 0);
}

/**
 *
 */
static zend_function_entry openssl_pkcs_p7s_methods[] = {
    PHP_ME(openssl_pkcs7, __construct, arginfo_openssl_pkcs_construct, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL|ZEND_ACC_CTOR)
    //PHP_ME(openssl_pkcs7, getSignature, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL|ZEND_ACC_CTOR)
    //PHP_ME(openssl_pkcs7, getContent, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL|ZEND_ACC_CTOR)
    //PHP_ME(openssl_pkcs7, verify, arginfo_openssl_pkcs_verify, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL|ZEND_ACC_CTOR)
    {NULL, NULL, NULL}
};

void openssl_pkcs_init_p7s(TSRMLS_D) {
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "Openssl\\P7s", openssl_pkcs_p7s_methods);
    openssl_pkcs_p7s_ce = zend_register_internal_class(&ce TSRMLS_CC);
    // flags
    openssl_pkcs_p7s_ce->ce_flags |= ZEND_ACC_FINAL_CLASS;
    // attributes
    zend_declare_property_null(openssl_pkcs_p7s_ce, "signatures", sizeof("signatures")-1, ZEND_ACC_PRIVATE TSRMLS_CC);
    //zend_declare_property_null(openssl_pkcs_p7s_ce, "content", sizeof("content")-1, ZEND_ACC_PRIVATE TSRMLS_CC);
    zend_declare_property_null(openssl_pkcs_p7s_ce, "isDetached", sizeof("isDetached")-1, ZEND_ACC_PRIVATE TSRMLS_CC);
}

/**
 *
 */
void updatePropertySignatures(void * object, PKCS7 * p7s) {
    int index;
    int numSignerInfo;
    STACK_OF(PKCS7_SIGNER_INFO) * p7sSignersInfo;
    PKCS7_SIGNER_INFO * p7sSignerInfo;
    zval * signatures;
    zval * signature;
    zval * signatureDatetime;
    zval * signatureSigner;

    MAKE_STD_ZVAL(signatures);
    array_init(signatures);

    if (getSignersInfo(p7s, &p7sSignersInfo) == EXIT_FAILURE) {
        zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Could not read signer info.", 0 TSRMLS_CC);
    }

    if (getSignersInfoCount(p7sSignersInfo, &numSignerInfo) == EXIT_FAILURE) {
        zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Could not count signer info.", 0 TSRMLS_CC);
    }

    for (index = 0; index < numSignerInfo; ++index) {
        if (getSignerInfo(p7sSignersInfo, &index, &p7sSignerInfo) == EXIT_FAILURE) {
            zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Could not read signer info.", 0 TSRMLS_CC);
        }

        MAKE_STD_ZVAL(signature);
        array_init(signature);

        if (getSignatureDatetime(p7sSignerInfo, &signatureDatetime) == EXIT_FAILURE) {
            zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Could not read signature datetime info.", 0 TSRMLS_CC);
        }
        add_assoc_zval(signature, "datetime", signatureDatetime);

        if (getSignatureSigner(p7sSignerInfo, &signatureSigner) == EXIT_FAILURE) {
            zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Could not read signature signer x509.", 0 TSRMLS_CC);
        }
        add_assoc_zval(signature, "signer", signatureSigner);

        add_next_index_zval(signatures, signature);
    }

    zend_update_property(openssl_pkcs_p7s_ce, object, "signatures", sizeof("signatures")-1, signatures TSRMLS_CC);
}

void updatePropertyIsDetached(void * object, int value) {
    zval * isDetachedAttribute;
    MAKE_STD_ZVAL(isDetachedAttribute);
    ZVAL_BOOL(isDetachedAttribute, value);
    zend_update_property(openssl_pkcs_p7s_ce, object, "isDetached", sizeof("isDetached")-1, isDetachedAttribute);
}

/**
 *
 */
int getSignatureDatetime(PKCS7_SIGNER_INFO * p7sSignerInfo, zval ** signatureDatetime) {
    unsigned char * datetime;
    zval * param1;
    zval * param2;

    if (getSignatureDatetimeString(p7sSignerInfo, &datetime) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    MAKE_STD_ZVAL(param1);
    ZVAL_STRING(param1, "ymdHisZ", 1);
    MAKE_STD_ZVAL(param2);
    ZVAL_STRING(param2, datetime, 1);

    if (zend_call_method(NULL, php_date_get_date_ce(), NULL, "createfromformat", strlen("createFromFormat"), &(*signatureDatetime), 2, param1, param2 TSRMLS_CC) == NULL) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int getSignatureSigner(PKCS7_SIGNER_INFO * p7sSignerInfo, zval ** signatureSigner) {
    zval * x509Param;
    zend_class_entry * x509CE = php_openssl_pkcs_get_x509_ce();

    MAKE_STD_ZVAL(*signatureSigner);
    object_init_ex(*signatureSigner, x509CE);

    MAKE_STD_ZVAL(x509Param);
    ZVAL_STRING(x509Param, "/var/www/pkcs/certificate.crt", 1);

    if (zend_call_method(signatureSigner, x509CE, &(x509CE)->constructor, ZEND_STRL(x509CE->constructor->common.function_name), NULL, 1, x509Param, NULL TSRMLS_CC) == FAILURE) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
