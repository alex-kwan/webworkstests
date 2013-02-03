/*
* Copyright (c) 2011-2012 Research In Motion Limited.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <bps/bps.h>
#include <bps/event.h>
#include <bps/navigator.h>
#include <bps/screen.h>
#include <fcntl.h>
#include <bps/vibration.h>
#include <camera/camera_api.h>
#include <iostream>
#include <zxing/common/GreyscaleLuminanceSource.h>
#include <zxing/common/HybridBinarizer.h>
#include <zxing/qrcode/QRCodeReader.h>
#include <zxing/MultiFormatReader.h>
using namespace zxing;
using namespace zxing::qrcode;
static bool shutdown;
screen_window_t vf_win = NULL;
static screen_context_t screen_ctx;
void errz(camera_error_t err)
{
switch (err) {
case CAMERA_EAGAIN:
fprintf(stderr,"The specified camera was not available. Try again.\n");
break;
case CAMERA_EINVAL:
fprintf(stderr,"The camera call failed because of an invalid parameter.\n");
break;
case CAMERA_ENODEV:
fprintf(stderr, "No such camera was found.\n");
break;
case CAMERA_EMFILE:
fprintf(stderr,"The camera called failed because of a file table overflow.\n");
break;
case CAMERA_EBADF:
fprintf(stderr,"Indicates that an invalid handle to a @c camera_handle_t value was used.\n");
break;
case CAMERA_EACCESS:
fprintf(stderr,"Indicates that the necessary permissions to access the camera are not available.\n");
break;
case CAMERA_EBADR:
fprintf(stderr,"Indicates that an invalid file descriptor was used.\n");
break;
case CAMERA_ENOENT:
fprintf(stderr,"Indicates that the access a file or directory that does not exist.\n");
break;
case CAMERA_ENOMEM:
fprintf(stderr, "Indicates that memory allocation failed.\n");
break;
case CAMERA_EOPNOTSUPP:
fprintf(stderr,
"Indicates that the requested operation is not supported.\n");
break;
case CAMERA_ETIMEDOUT:
fprintf(stderr,"Indicates an operation on the camera is already in progress. In addition, this error can indicate that an error could not be completed because i was already completed. For example, if you called the @c camera_stop_video() function but the camera had already stopped recording video, this error code would be returned.\n");
break;
case CAMERA_EALREADY:
fprintf(stderr,
"Indicates an operation on the camera is already in progress. In addition,this error can indicate that an error could not be completed because it was already completed. For example, if you called the @c camera_stop_video() function but the camera had already stopped recording video, this error code would be returned.\n");
break;
case CAMERA_EUNINIT:
fprintf(stderr,"Indicates that the Camera Library is not initialized.\n");
break;
case CAMERA_EREGFAULT:
fprintf(stderr,"Indicates that registration of a callback failed.\n");
break;
case CAMERA_EMICINUSE:
fprintf(stderr,"Indicates that it failed to open because microphone is already in use.\n");
break;
}
}

static void
handle_screen_event(bps_event_t *event)
{
    int screen_val;

    screen_event_t screen_event = screen_event_get_event(event);
    screen_get_event_property_iv(screen_event, SCREEN_PROPERTY_TYPE, &screen_val);

    switch (screen_val) {
    case SCREEN_EVENT_MTOUCH_TOUCH:
        fprintf(stderr,"Touch event");
        break;
    case SCREEN_EVENT_MTOUCH_MOVE:
        fprintf(stderr,"Move event");
        break;
    case SCREEN_EVENT_MTOUCH_RELEASE:
        fprintf(stderr,"Release event");
        break;
    case SCREEN_EVENT_CREATE:
            if (screen_get_event_property_pv(screen_event, SCREEN_PROPERTY_WINDOW, (void **)&vf_win) == -1) {
                perror("screen_get_event_property_pv(SCREEN_PROPERTY_WINDOW)");
            } else {
                fprintf(stderr,"viewfinder window found!\n");
                // mirror viewfinder if this is the front-facing camera
             //   int i = (shouldmirror?1:0);
                int i = 1;
                screen_set_window_property_iv(vf_win, SCREEN_PROPERTY_MIRROR, &i);
                // place viewfinder in front of the black application background window
                // child window z-orders are relative to their parent, not absolute.
                i = +1;
                screen_set_window_property_iv(vf_win, SCREEN_PROPERTY_ZORDER, &i);
                // make viewfinder window visible
                i = 1;
                screen_set_window_property_iv(vf_win, SCREEN_PROPERTY_VISIBLE, &i);
                screen_flush_context(screen_ctx, 0);
                // we should now have a visible viewfinder
                //touch = false;
             //   state = STATE_VIEWFINDER;
            }
            break;
    default:
        break;
    }
    fprintf(stderr,"\n");
}

static void
handle_navigator_event(bps_event_t *event) {
    switch (bps_event_get_code(event)) {
    case NAVIGATOR_SWIPE_DOWN:
        fprintf(stderr,"Swipe down event");
        break;
    case NAVIGATOR_EXIT:
        fprintf(stderr,"Exit event");
        shutdown = true;
        break;
    default:
        break;
    }
    fprintf(stderr,"\n");
}

static const char vf_group[] = "viewfinder_window_group";

static void
viewfinder_callback(camera_handle_t handle,
                    camera_buffer_t* buf,
                    void* arg)
{
	fprintf(stderr, "init now!\n");
	fprintf( stderr, "frame size = %d\n", buf->framesize);
	fprintf( stderr, "frame type = %d %d\n", buf->frametype,CAMERA_FRAMETYPE_NV12);
	// these values need to be updated. Find out how to extract this info from the camera buffer


	camera_frame_nv12_t* data = (camera_frame_nv12_t*)(&(buf->framedesc));
			int stride = data->stride;
			int width  = data->width;//0;
			int height = data->height;//0;
			fprintf(stderr, "setting up variables now! %d %d %d\n", stride, width, height);
            fprintf(stderr, "variables for setting up source %d %d %d %d\n"
            		, stride
            		, buf->framesize / stride
            		, width
            		, height
            		);

            try{


            	Ref<LuminanceSource> source(new GreyscaleLuminanceSource(buf->framebuf, width, height,0,245,700, 700));


			fprintf(stderr, "2setting up variables now!\n");
			Ref<Binarizer> binarizer(new HybridBinarizer(source));
			fprintf(stderr, "3setting up variables now!\n");
			Ref<BinaryBitmap> bitmap(new BinaryBitmap(binarizer));
			fprintf(stderr, "4setting up variables now!\n");
			Ref<Result> result;
			fprintf(stderr, "creating reader now!\n");

//			Ref<MultiFormatReader> m_reader = Ref<MultiFormatReader>(new MultiFormatReader());
			            DecodeHints *hints = new DecodeHints();

//
			QRCodeReader *reader = new QRCodeReader();
		//	DecodeHints hints;
			hints->addFormat(BarcodeFormat_QR_CODE);
			hints->addFormat(BarcodeFormat_EAN_8);
			hints->addFormat(BarcodeFormat_EAN_13);
			hints->addFormat(BarcodeFormat_UPC_A);
			hints->addFormat(BarcodeFormat_UPC_E);
			hints->addFormat(BarcodeFormat_DATA_MATRIX);
			hints->addFormat(BarcodeFormat_CODE_128);
			hints->addFormat(BarcodeFormat_CODE_39);
			hints->addFormat(BarcodeFormat_ITF);
			hints->addFormat(BarcodeFormat_AZTEC);
//			m_reader->setHints(*hints);

			// If the preview buffer is in landscape, we can rotate out bitmap to let us scan 1D codes
//			if (m_landscapePreviewFrames) {
//				Ref<BinaryBitmap> rotated = bitmap->rotateCounterClockwise();
//				result = reader->decode(rotated, hints);
//			}
//			else {
		fprintf(stderr, "decoding now!\n");
		//result = m_reader->decode(bitmap);
		result = reader->decode(bitmap, *hints);
		//	}

			// get the result of decoding the image
			std::string newBarcodeData = result->getText()->getText().data();
			vibration_request(1000, 100);
            } catch (const std::exception& ex) {
                                  	fprintf( stderr, "error occured : \%s n", ex.what());
                                  }

		}

static void
handle_event()
{
    int domain;

    bps_event_t *event = NULL;
    if (BPS_SUCCESS != bps_get_event(&event, -1)) {
        fprintf(stderr, "bps_get_event() failed\n");
        return;
    }
    if (event) {
        domain = bps_event_get_domain(event);
        if (domain == navigator_get_domain()) {
            handle_navigator_event(event);
        } else if (domain == screen_get_domain()) {
            handle_screen_event(event);
        }
    }
}




int
main(int argc, char **argv)
{




    const int usage = SCREEN_USAGE_NATIVE;


    screen_window_t screen_win;
    screen_buffer_t screen_buf = NULL;
    int rect[4] = { 0, 0, 0, 0 };

    /* Setup the window */
    screen_create_context(&screen_ctx, 0);
    screen_create_window(&screen_win, screen_ctx);
    screen_create_window_group(screen_win, vf_group);
    screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_USAGE, &usage);
    screen_create_window_buffers(screen_win, 1);


    screen_get_window_property_pv(screen_win, SCREEN_PROPERTY_RENDER_BUFFERS, (void **)&screen_buf);
    screen_get_window_property_iv(screen_win, SCREEN_PROPERTY_BUFFER_SIZE, rect+2);

    /* Fill the screen buffer with blue */
//    int attribs[] = { SCREEN_BLIT_COLOR, 0xff0000ff, SCREEN_BLIT_END };
//    screen_fill(screen_ctx, screen_buf, attribs);
//    screen_post_window(screen_win, screen_buf, 1, rect, 0);


    // Fill the window with black.
    int attribs[] = { SCREEN_BLIT_COLOR, 0x00000000, SCREEN_BLIT_END };
    screen_fill(screen_ctx, screen_buf, attribs);
    screen_post_window(screen_win, screen_buf, 1, rect, 0);

    // Signal bps library that navigator and screen events will be requested.
    bps_initialize();
    //main_bps_chid = bps_channel_get_active();
    screen_request_events(screen_ctx);
    navigator_request_events(0);


    camera_handle_t mCameraHandle;
           camera_unit_t mCameraUnit;
           mCameraHandle = CAMERA_HANDLE_INVALID;
           camera_error_t err;
              // Open the specified camera.


             err = camera_open(
           		  CAMERA_UNIT_REAR
           		,CAMERA_MODE_RW | CAMERA_MODE_ROLL
           		,&mCameraHandle);
             if ( err != CAMERA_EOK){
            	 fprintf(stderr, " error1 = %d\n ", err);
             }
             int numSupported = 0;
             camera_frametype_t types[30];
             camera_error_t t;
             t = camera_get_photovf_frame_types( mCameraHandle, 0 , &numSupported , types);
             if ( t != CAMERA_EOK){
			  fprintf(stderr,"We have a problem with retrieving the frame types \n");
			  errz( t ) ;
			  return EIO;
			 }

             for(int i = 0; i < numSupported; i++){
            	 fprintf(stderr, " %d \n ",types[i]);
             }



             t =  camera_set_photovf_property( mCameraHandle,

                                                    CAMERA_IMGPROP_WIN_GROUPID, vf_group
                                                    ,CAMERA_IMGPROP_WIN_ID, "my_viewfinder"
												//    ,CAMERA_IMGPROP_FORMAT, CAMERA_FRAMETYPE_JPEG
              	  	  	  	  	  	  	  );
             fprintf(stderr,"Moving on to starting the viewfinder\n");
             if ( t != CAMERA_EOK) {
                    	   fprintf(stderr, "Ran into a strange issue when starting up the camera properties \n");
                    	   errz( t ) ;
            			 return EIO;
            		 }
             t = camera_start_photo_viewfinder( mCameraHandle
	   	   	   	   	   	  ,&viewfinder_callback
         	              , NULL//   ,&status_callback
         	  		 //,NULL
             		 //,NULL
         	              ,NULL
         	           );
           if ( t != CAMERA_EOK) {
        	   fprintf(stderr, "Ran into a strange issue when starting up the camera viewfinder\n");
        	   errz( t ) ;
			 return EIO;
		 }
    while (!shutdown) {
        /* Handle user input */
        handle_event();

    }

    /* Clean up */
    screen_stop_events(screen_ctx);
    bps_shutdown();
    screen_destroy_window(screen_win);
    screen_destroy_context(screen_ctx);
    return 0;
}

