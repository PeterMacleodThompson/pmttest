/******************  SDL2init.c   ***************************/
/*DEBUG 
  X86 compile:  gcc -o X86 SDLcolortest.c  -lSDL2 -lSDL2_image
  ARM compile:  
	export PATH=$PATH:$HOME/bbb2018/buildroot/output/host/bin ## for compiler
	arm-linux-gnueabihf-gcc -g -o ARM -D BBB  SDLcolortest.c -L /home/peter/bbb2018/buildroot/output/staging/usr/lib/ -lSDL2 -lSDL2_image
*/

/* init SDL Video and GLOBAL Window, SDL GLOBAL Renderer, and SDL_Image file loading 
  must gcc -D BBB (beagleboneblack) else X86 version will be compiled
*/

#define DEBUG   //conditional compilation for debugging

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h> 
#include <SDL2/SDL_log.h>
#include <stdio.h>

#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 500


#define TRUE 1
#define FALSE 0

/* GLOBAL VARIABLES FOR WINDOW */
SDL_Window* globalwindow;			//Display window we'll be rendering to 
SDL_Renderer* globalrenderer; 		//The window renderer
SDL_Texture* globaltexture;			//texture for display window 




int initSDL2()
{
    int success = TRUE;

	/* informational debugging variables */
	SDL_RendererInfo info;
	int r, i;

	/* print machine video driver information */
	r = SDL_GetNumVideoDrivers();
	printf("Number of Video Drivers = %d\n", r);
	i=0;
	while(i < r)
	{
		printf("  video driver %d = %s\n", i, SDL_GetVideoDriver(i) ); 
		i++;
	}

	/* debug SDL2 with messages to stderr */
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
	SDL_Log("SDL log is verbose\n");


	//Initialize SDL2 - video only
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		success = FALSE;
	}
	else  
		printf("Current Video Driver = %s\n", SDL_GetCurrentVideoDriver()  ); 




	//Create GLOBAL window
	if(success == TRUE)
	{
		globalwindow = SDL_CreateWindow( "hiker!!", SDL_WINDOWPOS_UNDEFINED,
			 SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, 0 );
		if( globalwindow == NULL )
		{
			printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
			success = FALSE;
		}
	}

		
	/*Create GLOBAL renderer, use "software renderer" for now
	see SDL_render.c in SDL2sources */
	if(success == TRUE)
	{	
		/* print available renderer drivers */
		r = SDL_GetNumRenderDrivers();
		printf("NumRenderDrivers = %d\n", r );
		i = 0;
		while(i < r)
		{
			if ( SDL_GetRenderDriverInfo(i,&info) == 0 ) {
				printf("  render driver %d = %s ", i, info.name);
				printf("    flags(hex) = %x\n", info.flags);
				printf("    num texture formats = %d\n",
								 info.num_texture_formats);
				printf("    texture max width, height = %d, %d\n",
						info.max_texture_width, info.max_texture_height); 
			}
			i++;
		}

		/* set hintS for different path in SDL_render.c - see SDL2sources */
		SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
		SDL_SetHint(SDL_HINT_FRAMEBUFFER_ACCELERATION, 0);  


		/* initialize renderer */
		globalrenderer = SDL_CreateRenderer( globalwindow, -1, 0);
		if( globalrenderer == NULL )
		{
			printf( "Renderer could not be created! SDL Error: %s\n",
					SDL_GetError() );
			success = FALSE;
		}
		else
		{
			/* print selected renderer */
			SDL_GetRendererInfo(globalrenderer, &info);
			printf(" Selected Renderer = %s\n", info.name);
		}
	}

	//Initialize PNG, TIF, JPG loading
	if(success == TRUE)
	{
		int imgFlags = IMG_INIT_PNG | IMG_INIT_TIF | IMG_INIT_JPG;
		if( !( IMG_Init( imgFlags ) & imgFlags ) )   {
			printf( "SDL_image could not initialize! SDL_image Error: %s\n",
							 IMG_GetError() );
			success = FALSE;
		}		
	}



	/*  create global textures 
		for X86 use  SDL_PIXELFORMAT_ARGB8888,
		for beaglebone black use SDL_PIXELFORMAT_RGB565*/
	if(success == TRUE)
	{
		#if BBB
		globaltexture = SDL_CreateTexture(globalrenderer,
                               SDL_PIXELFORMAT_RGB888,
                               SDL_TEXTUREACCESS_STREAMING,
                               SCREEN_WIDTH, SCREEN_HEIGHT);
		#else   /* default back to X86 */
		globaltexture = SDL_CreateTexture(globalrenderer,
                               SDL_PIXELFORMAT_ARGB8888,
                               SDL_TEXTUREACCESS_STREAMING,
                               SCREEN_WIDTH, SCREEN_HEIGHT);
		#endif
		if( globaltexture == NULL )
			{
				printf( "Texture could not be created! SDL Error: %s\n", 
						SDL_GetError() );
				success = FALSE;
			}
		
		SDL_SetRenderDrawColor(globalrenderer,
				255, 0, 0, 255); //for drawing, clearing; ==>Red,Opaque 
		SDL_SetRenderDrawBlendMode(globalrenderer, 
				SDL_BLENDMODE_BLEND); //FIXME 
		//FIXME remove the above line after alpha works - installed for alpha

	}

	printf("SDL2init complete\n\n\n");
	return(success);
}

void closeSDL2()
{

	//Destroy window
	SDL_DestroyWindow( globalwindow );
 	SDL_DestroyRenderer( globalrenderer );
 	SDL_DestroyTexture( globaltexture );

	globalwindow = NULL;
 	globalrenderer = NULL;
	//Quit SDL subsystems IMG_Load
	SDL_Quit();

}


#ifdef DEBUG
/******** main for testing **********/

int main( int argc, char *argv[])
{
	SDL_Surface *removeme;

	char inbuf[100];
    int err=0;
	int red, green, blue;

    if(argc != 4)  {
	    printf("need 3 arguments: red(0-155) green(0-255) blue(0-255)");
		exit(0);
	}
	sscanf(argv[1], "%d", &red);
	sscanf(argv[2], "%d", &green);
	sscanf(argv[3], "%d", &blue);
	printf("RED=%d, GREEN=%d, BLUE=%d\n", red, green, blue);

		

	if( initSDL2()  )
	{
		printf(" err = %d\n", err);

		//Create window working SDL_surface
		removeme = SDL_GetWindowSurface( globalwindow );


		err = SDL_FillRect( removeme, NULL, 
			SDL_MapRGB(removeme->format, red, green, blue ));
		if(err != 0)
			printf("SDL_FillRect Error %s\n", SDL_GetError()  );

		err = SDL_UpdateTexture( globaltexture, NULL, 
				removeme->pixels, removeme->pitch);	
		if(err != 0)
			printf("SDL_UpdateTexture Error %s\n", SDL_GetError()  );

		err = SDL_RenderClear(globalrenderer);
		err = SDL_RenderCopy(globalrenderer, globaltexture, NULL, NULL);
		SDL_RenderPresent(globalrenderer);
		if(err != 0)
			printf("SDL_UpdateTexture Error %s\n", SDL_GetError()  );

		//Wait 4 seconds
		SDL_Delay( 4000 );

		SDL_FreeSurface( removeme );		
		closeSDL2();
	}
}


#endif

