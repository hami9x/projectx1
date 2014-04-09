#ifndef TEXTURE_H_INCLUDED
#define TEXTURE_H_INCLUDED
//Class developed from LazyFoo's tutorial
#include <SDL.h>
#include <string>

//Texture wrapper class
class XxTexture
{
	public:
		//Initializes variables
		XxTexture();

		//Deallocates memory
		~XxTexture();

		//Loads image at specified path
		bool loadFromFile( SDL_Renderer *r, std::string path );

		//Deallocates texture
		void free();

		//Set color modulation
		void setColor( Uint8 red, Uint8 green, Uint8 blue );

		//Set blending
		void setBlendMode( SDL_BlendMode blending );

		//Set alpha modulation
		void setAlpha( Uint8 alpha );

		//Renders texture at given point
		void render( SDL_Renderer *r, int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE );

		bool updateSurface(SDL_Renderer *r, SDL_Surface* surface);

		//Gets image dimensions
		int width();
		int height();

	private:
		//The actual hardware texture
		SDL_Texture* mTexture;

		//Image dimensions
		int mWidth;
		int mHeight;
		SDL_Renderer *mRenderer;
};

#endif // TEXTURE_H_INCLUDED
