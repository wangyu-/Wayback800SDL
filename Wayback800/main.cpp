#include <SDL2/SDL.h>
#include "NekoDriver.h"
#include <iostream>
#include <map>
using namespace std;

extern TScreenBuffer renderLCDBuffer;

const uint32_t SCREEN_WIDTH=160;
const uint32_t SCREEN_HEIGHT=80;
const uint32_t LINE_SIZE=2;

const uint32_t FRAME_RATE=100;
const uint32_t FRAME_INTERVAL= (1000u/FRAME_RATE);

SDL_Renderer* renderer;

static uint8_t (&lcd_buf)[SCREEN_WIDTH * SCREEN_HEIGHT / 8] =renderLCDBuffer.fPixel;

void dummy_call_back(){
        //printf("i am runninging!\n");
}

bool InitEverything() {
  if (SDL_Init(SDL_INIT_EVERYTHING) == -1) {
    std::cout << " Failed to initialize SDL : " << SDL_GetError() << std::endl;
    return false;
  }
  SDL_Window* window =
    SDL_CreateWindow("WQX", 0, 40, LINE_SIZE * SCREEN_WIDTH, LINE_SIZE * SCREEN_HEIGHT, 0);
  if (!window) {
    std::cout << "Failed to create window : " << SDL_GetError() << std::endl;
    return false;
  }
  renderer = SDL_CreateRenderer(window, -1, 0);
  if (!renderer) {
    std::cout << "Failed to create renderer : " << SDL_GetError() << std::endl;
    return false;
  }
  SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH * LINE_SIZE, SCREEN_HEIGHT * LINE_SIZE);
  
  return true;
}

void Render() {
  SDL_RenderClear(renderer);
  SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
    SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

  unsigned char* bytes = nullptr;
  int pitch = 0;
  static const SDL_Rect source = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
  SDL_LockTexture(texture, &source, reinterpret_cast<void**>(&bytes), &pitch);
  static const unsigned char on_color[4] = { 255, 0, 0, 0 };
  static const unsigned char off_color[4] = { 255, 255, 255, 255 };
  static const size_t color_size = sizeof(on_color);
  for (int i = 0; i < sizeof(lcd_buf); ++i) {
    for (int j = 0; j < 8; ++j) {
      bool pixel = (lcd_buf[i] & (1 << (7 - j))) != 0;
      memcpy(bytes, pixel ? on_color : off_color, color_size);
      bytes += color_size;
    }
  }
  SDL_UnlockTexture(texture);
  static const SDL_Rect destination =
    { 0, 0, SCREEN_WIDTH * LINE_SIZE, SCREEN_HEIGHT * LINE_SIZE };
  SDL_RenderCopy(renderer, texture, &source, &destination);
  SDL_RenderPresent(renderer);
  SDL_DestroyTexture(texture);
}


int main()
{
        InitEverything();
        theNekoDriver = new TNekoDriver();
        theNekoDriver->SetLCDBufferChangedCallback(&dummy_call_back);
        theNekoDriver->RunDemoBin("");
        SDL_Event event;
        bool loop = true;
        while(loop){
                while (SDL_PollEvent(&event)) {
                        if ( event.type == SDL_QUIT ) {
                                loop = false;
                        } else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                                bool key_down = (event.type == SDL_KEYDOWN);

                                //try to consolidate multiple key shoot into one
                                //not sure if necessary. But it's helpful for debug
                        //mp[event.key.keysym.sym]= key_down;
                                //for(auto it=mp.begin();it!=mp.end();it++){
                                //handle_key(it->first,it->second);
                                //}
                        }
                }
                Render();
        }
        return 0;
}
