/*
@author: Vladya
*/

#include "bassWrapper.h"
#include <SDL.h>

float *dataBuffer = NULL;

HSTREAM handler = 0;
static BOOL initialized = FALSE;
static char *currentFileData = NULL;


static int init(void) {
    /*
    Initialized virtual channel.
    Return error code.
    0 - No error.
    -2 - if device is already initialized.
    */
    if (initialized) {
        return (-2);
    };
    const DWORD deviceId = 0;  // No sound. Decodng only.
    BOOL initSuccesful = BASS_Init(deviceId, FREQUENCY, 0, 0, NULL);
    if (!initSuccesful) {
        return getErrorCode();
    };
    BOOL setSuccesful = BASS_SetDevice(deviceId);
    if (!setSuccesful) {
        int errorCode = getErrorCode();
        BASS_Free();
        return errorCode;
    };
    initialized = TRUE;
    return 0;
};

static int freeStream(void) {
    /*
    Return 0 if succesful, error code if not.
    -2 - if device not initialized.
    */
    if (!initialized) {  // Not initialized.
        return (-2);
    };
    if (handler) {
        BASS_StreamFree(handler);
        handler = 0;
    };
    BOOL isSuccessful = BASS_Free();
    if (isSuccessful) {
        dataBuffer = realloc(dataBuffer, 0);
        currentFileData = realloc(currentFileData, 0);
        initialized = FALSE;
        return 0;
    };
    return getErrorCode();
};

static int setFile(const SDL_RWops *fileObject) {
    /*
    Return error code.
    0 - No error.
    -2 - Device not initialized.
    -4 - memory error.
    -1 - some other error.
    */
    if (!initialized) {
        return (-2);
    };
    if (handler) {
        // Free old stream.
        BASS_ChannelLock(handler, TRUE);
        BASS_StreamFree(handler);
        handler = 0;
    };
    size_t fileSize = (size_t) fileObject->size(fileObject);
    if (fileSize == (-1)) {
        return (-1);
    };
    currentFileData = (char *) realloc(currentFileData, fileSize);
    if (!currentFileData) {
        return (-4);
    };
    int readNum = fileObject->read(fileObject, currentFileData, 1, fileSize);
    if (readNum == 0) {
        currentFileData = realloc(currentFileData, 0);
        return (-1);
    };
    HSTREAM _handler = BASS_StreamCreateFile(
        TRUE,
        currentFileData,
        0,
        ((QWORD) fileSize),
        (BASS_STREAM_DECODE|BASS_STREAM_PRESCAN)
    );
    if (_handler == 0) {
        currentFileData = realloc(currentFileData, 0);
        return getErrorCode();
    };
    handler = _handler;
    return 0;
};

static int updateBufferFromPos(const int posInMs) {
    /*
    Return error code.
    0 - No error.
    -2 - Not initialized.
    -4 - Memory error.
    */
    if ((!initialized) || (!handler)) {
        return (-2);
    };

    BASS_ChannelLock(handler, TRUE);

    QWORD posInBytes = BASS_ChannelSeconds2Bytes(
        handler,
        (((double) posInMs) / 1000.)
    );
    if (posInBytes == (-1)) {
        int errorCode = getErrorCode();
        BASS_ChannelLock(handler, FALSE);
        return errorCode;
    };
    BOOL setPosSuccesful = BASS_ChannelSetPosition(
        handler,
        posInBytes,
        BASS_POS_BYTE
    );
    if (!setPosSuccesful) {
        int errorCode = getErrorCode();
        BASS_ChannelLock(handler, FALSE);
        return errorCode;
    };

    dataBuffer = (float *) realloc(
        dataBuffer,
        (sizeof(float) * BUFFER_LEN_FFT_ARRAY)
    );
    if (!dataBuffer) {
        BASS_ChannelLock(handler, FALSE);
        return (-4);
    };
    DWORD readedSamplesNum = BASS_ChannelGetData(
        handler,
        dataBuffer,
        (BUFFER_LEN_FLAG|BASS_DATA_FFT_REMOVEDC)
    );
    if (readedSamplesNum == (-1)) {
        dataBuffer = realloc(dataBuffer, 0);
        int errorCode = getErrorCode();
        BASS_ChannelLock(handler, FALSE);
        return errorCode;
    };

    BASS_ChannelLock(handler, FALSE);
    return 0;
};
