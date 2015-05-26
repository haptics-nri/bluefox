#ifdef _MSC_VER // is Microsoft compiler?
#   if _MSC_VER < 1300  // is 'old' VC 6 compiler?
#       pragma warning( disable : 4786 ) // 'identifier was truncated to '255' characters in the debug information'
#   endif // #if _MSC_VER < 1300
#endif // #ifdef _MSC_VER
#include <mvIMPACT_CPP/mvIMPACT_acquire.h>
#ifdef _WIN32
#   include <mvDisplay/Include/mvIMPACT_acquire_display.h>
using namespace mvIMPACT::acquire::display;
#endif // #ifdef _WIN32
#include <iostream>
#include <string>

using namespace std;
using namespace mvIMPACT::acquire;

#ifdef linux
#   define NO_DISPLAY
#   include <stdint.h>
#   include <stdio.h>
typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef int32_t LONG;
typedef bool BOOLEAN;

#   ifdef __GNUC__
#       define BMP_ATTR_PACK __attribute__((packed)) __attribute__ ((aligned (2)))
#   else
#       define BMP_ATTR_PACK
#   endif // #ifdef __GNUC__
#ifndef NO_DISPLAY
    // initialise display window
    ImageDisplayWindow displayWindow( "mvIMPACT_acquire sample" );
    displayWindow.GetImageDisplay().SetImage( pRequest );
    displayWindow.GetImageDisplay().Update();
#endif // NO_DISPLAY

typedef struct tagRGBQUAD
{
    BYTE    rgbBlue;
    BYTE    rgbGreen;
    BYTE    rgbRed;
    BYTE    rgbReserved;
} BMP_ATTR_PACK RGBQUAD;

typedef struct tagBITMAPINFOHEADER
{
    DWORD  biSize;
    LONG   biWidth;
    LONG   biHeight;
    WORD   biPlanes;
    WORD   biBitCount;
    DWORD  biCompression;
    DWORD  biSizeImage;
#ifndef NO_DISPLAY
    // initialise display window
    ImageDisplayWindow displayWindow( "mvIMPACT_acquire sample" );
    displayWindow.GetImageDisplay().SetImage( pRequest );
    displayWindow.GetImageDisplay().Update();
#endif // NO_DISPLAY
    LONG   biXPelsPerMeter;
    LONG   biYPelsPerMeter;
    DWORD  biClrUsed;
    DWORD  biClrImportant;
} BMP_ATTR_PACK BITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef struct tagBITMAPFILEHEADER
{
    WORD    bfType;
    DWORD   bfSize;
    WORD    bfReserved1;
    WORD    bfReserved2;
    DWORD   bfOffBits;
} BMP_ATTR_PACK BITMAPFILEHEADER, *PBITMAPFILEHEADER;
#else
#   undef NO_DISPLAY
#endif

//-----------------------------------------------------------------------------
int SaveBMP( const string& filename, char* pdata, int XSize, int YSize, int pitch, int bitsPerPixel )
//------------------------------------------------------------------------------
{
    static const WORD PALETTE_ENTRIES = 256;

    if( pdata )
    {
        FILE* pFile = fopen( filename.c_str(), "wb" );
        if( pFile )
        {
            BITMAPINFOHEADER    bih;
            BITMAPFILEHEADER    bfh;
            WORD                linelen = static_cast<WORD>( ( XSize * bitsPerPixel + 31 ) / 32 * 4 );  // DWORD aligned
            int                 YPos;
            int                 YStart = 0;

            memset( &bfh, 0, sizeof( BITMAPFILEHEADER ) );
            memset( &bih, 0, sizeof( BITMAPINFOHEADER ) );
            bfh.bfType          = 0x4d42;
            bfh.bfSize          = sizeof( bih ) + sizeof( bfh ) + sizeof( RGBQUAD ) * PALETTE_ENTRIES + static_cast<LONG>( linelen ) * static_cast<LONG>( YSize );
            bfh.bfOffBits       = sizeof( bih ) + sizeof( bfh ) + sizeof( RGBQUAD ) * PALETTE_ENTRIES;
            bih.biSize          = sizeof( bih );
            bih.biWidth         = XSize;
            bih.biHeight        = YSize;
            bih.biPlanes        = 1;
            bih.biBitCount      = static_cast<WORD>( bitsPerPixel );
            bih.biSizeImage     = static_cast<DWORD>( linelen ) * static_cast<DWORD>( YSize );

            if( ( fwrite( &bfh, sizeof( bfh ), 1, pFile ) == 1 ) && ( fwrite( &bih, sizeof( bih ), 1, pFile ) == 1 ) )
            {
                RGBQUAD rgbQ;
                for( int i = 0; i < PALETTE_ENTRIES; i++ )
                {
                    rgbQ.rgbRed      = static_cast<BYTE>( i );
                    rgbQ.rgbGreen    = static_cast<BYTE>( i );
                    rgbQ.rgbBlue     = static_cast<BYTE>( i );
                    rgbQ.rgbReserved = static_cast<BYTE>( 0 );
                    fwrite( &rgbQ, sizeof( rgbQ ), 1, pFile );
                }
		
		// BGR -> RGB
		char r, g, b;
		for (int y = 0; y < YSize; ++y)
		{
			for (int x = 0; x < XSize; ++x)
			{
				r = pdata[y*pitch + x*3 + 0];
				g = pdata[y*pitch + x*3 + 1];
				b = pdata[y*pitch + x*3 + 2];
				pdata[y*pitch + x*3 + 0] = b;
				pdata[y*pitch + x*3 + 1] = g;
				pdata[y*pitch + x*3 + 2] = r;
			}
		}


                for( YPos = YStart + YSize - 1; YPos >= YStart; YPos-- )
                {
                    if( fwrite( &pdata[YPos * pitch], linelen, 1, pFile ) != 1 )
                    {
                        cout << "SaveBmp: ERR_WRITE_FILE: " << filename << endl;
                    }
                }
            }
            else
            {
                cout << "SaveBmp: ERR_WRITE_FILE: " << filename << endl;
            }
            fclose( pFile );
        }
        else
        {
            cout << "SaveBmp: ERR_CREATE_FILE: " << filename << endl;
        }
    }
    else
    {
        cout << "SaveBmp: ERR_DATA_INVALID:" << filename << endl;
    }
    return 0;
}

//-----------------------------------------------------------------------------
int main( int /*argc*/, char* /*argv*/[] )
//-----------------------------------------------------------------------------
{
    DeviceManager devMgr;
    Device* pDev = devMgr.getDeviceByProduct("mvBlueFOX3-1020C", 0);
    if( !pDev )
    {
        cerr << "logger: no camera found! Exiting" << endl;
        return 1;
    }

    try
    {
        pDev->open();
    }
    catch( const ImpactAcquireException& e )
    {
        // this e.g. might happen if the same device is already opened in another process...
        cerr << "An error occurred while opening the device " << pDev->serial.read() << " (error code: " << e.getErrorCode() << "). Exiting" << endl;
        return 2;
    }

    cout << "The device " << pDev->serial.read() << " has been opened." << endl;
    FunctionInterface fi( pDev );

    // send a request to the default request queue of the device and wait for the result.
    fi.imageRequestSingle();
    // Start the acquisition manually if this was requested(this is to prepare the driver for data capture and tell the device to start streaming data)
    if( pDev->acquisitionStartStopBehaviour.read() == assbUser )
    {
        TDMR_ERROR result = DMR_NO_ERROR;
        if( ( result = static_cast<TDMR_ERROR>( fi.acquisitionStart() ) ) != DMR_NO_ERROR )
        {
            cerr << "'FunctionInterface.acquisitionStart' returned with an unexpected result: " << result
                 << "(" << ImpactAcquireException::getErrorCodeAsString( result ) << ")" << endl;
	    return 3;
        }
    }
    const int iMaxWaitTime_ms = 500;
    // wait for results from the default capture queue
    int requestNr = fi.imageRequestWaitFor( iMaxWaitTime_ms );

    // check if the image has been captured without any problems
    if( !fi.isRequestNrValid( requestNr ) )
    {
        // If the error code is -2119(DEV_WAIT_FOR_REQUEST_FAILED), the documentation will provide
        // additional information under TDMR_ERROR in the interface reference
        cerr << "imageRequestWaitFor failed (" << requestNr << ", " << ImpactAcquireException::getErrorCodeAsString( requestNr ) << ")"
             << ", timeout value too small?" << endl;
        return 4;
    }

    const Request* pRequest = fi.getRequest( requestNr );
    if( !pRequest->isOK() )
    {
        cerr << "Error: " << pRequest->requestResult.readS() << endl;
        return 5;
    }

    // everything went well. Display the result

    const string filename( "single.bmp" );
    cerr << "Storing the image as \"" << filename << "\"" << endl;
    SaveBMP( filename, reinterpret_cast<char*>( pRequest->imageData.read() ), pRequest->imageWidth.read(), pRequest->imageHeight.read(), pRequest->imageLinePitch.read(), pRequest->imagePixelPitch.read() * 8 );
    // unlock the buffer to let the driver know that you no longer need this buffer
    fi.imageRequestUnlock( requestNr );

    cerr << "Bye!" << endl;

    return 0;
}
