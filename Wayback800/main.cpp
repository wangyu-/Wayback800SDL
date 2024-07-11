#include <SDL2/SDL.h>
#include "NekoDriver.h"
#include <SDL_keycode.h>
#include <iostream>
#include <map>
#include "KeyItem.h"
#include "comm.h"
using namespace std;

extern TScreenBuffer renderLCDBuffer;

const uint32_t SCREEN_WIDTH=160;
const uint32_t SCREEN_HEIGHT=80;
const uint32_t LINE_SIZE=2;

SDL_Renderer* renderer;

static uint8_t (&lcd_buf)[SCREEN_WIDTH * SCREEN_HEIGHT / 8] =renderLCDBuffer.fPixel;

extern unsigned /*char*/ keypadmatrix[8][8];


   TKeyItem* item[8][8] = {
        NULL,       // P10, P30
        NULL,       // P11, P30
        new TKeyItem(18, NULL, NULL, "ON/OFF", {SDLK_F12}),        // GND, P30
        NULL,       // P??, P30
        NULL,       // P??, P30
        NULL,       // P??, P30
        NULL,       // P??, P30
        NULL,       // P??, P30
        
        new TKeyItem(0, "英汉", NULL, "汉英",{SDLK_F5}),          // P00, P30
        new TKeyItem(1, "名片", NULL, "通讯",{SDLK_F6}),          // P01, P30
        new TKeyItem(2, "计算", NULL, "换算",{SDLK_F7}),          // P02, P30
        new TKeyItem(3, "行程", NULL, "记事",{SDLK_F8}),          // P03, P30
        new TKeyItem(4, "资料", NULL, "游戏",{SDLK_F9}),          // P04, P30
        new TKeyItem(5, "时间", NULL, "其他",{SDLK_F10}),        // P05, P30
        new TKeyItem(6, "网络", NULL, NULL,{SDLK_F11}),        // P06, P30
        NULL,       // P07, P30
        
        new TKeyItem(50, "求助", NULL, NULL,{SDLK_LEFTBRACKET}),  // P00, P12
        new TKeyItem(51, "中英数", NULL, "SHIFT",{SDLK_RIGHTBRACKET}),   // P01, P12
        new TKeyItem(52, "输入法", NULL, "反查 CAPS",{SDLK_BACKSLASH}), // P02, P12
        new TKeyItem(53, "跳出", "AC", NULL, {SDLK_ESCAPE}),     // P03, P12
        new TKeyItem(54, "符\n号", "0", "继续", {SDLK_0}),           // P04, P12
        new TKeyItem(55, ".", ".", "-", {SDLK_PERIOD}),      // P05, P12
        new TKeyItem(56, "空格", "=", "✓", {SDLK_EQUALS}),       // P06, P12
        new TKeyItem(57, "←", "", NULL, {SDLK_LEFT,SDLK_BACKSPACE}),     // P07, P12
        
        new TKeyItem(40, "Z", "(", ")",{SDLK_z}),           // P00, P13
        new TKeyItem(41, "X", "π", "X!",{SDLK_x}),           // P01, P13
        new TKeyItem(42, "C", "EXP", "。'\"",{SDLK_c}),           // P02, P13
        new TKeyItem(43, "V", "C",NULL,{SDLK_v}),           // P03, P13
        new TKeyItem(44, "B", "1",NULL,{SDLK_b,SDLK_1}),           // P04, P13
        new TKeyItem(45, "N", "2",NULL,{SDLK_n,SDLK_2}),           // P05, P13
        new TKeyItem(46, "M", "3",NULL,{SDLK_m,SDLK_3}),           // P06, P13
        new TKeyItem(47, "⇞", "税",NULL,{SDLK_COMMA}),   // P07, P13
        
        new TKeyItem(30, "A", "log", "10x",{SDLK_a}),       // P00, P14
        new TKeyItem(31, "S", "ln", "ex",{SDLK_s}),       // P01, P14
        new TKeyItem(32, "D", "Xʸ", "y√x",{SDLK_d}),       // P02, P14
        new TKeyItem(33, "F", "√", "X\u00B2",{SDLK_f}),       // P03, P14
        new TKeyItem(34, "G", "4",NULL,{SDLK_g,SDLK_4}),       // P04, P14
        new TKeyItem(35, "H", "5",NULL,{SDLK_h,SDLK_5}),       // P05, P14
        new TKeyItem(36, "J", "6",NULL,{SDLK_j,SDLK_6}),       // P06, P14
        new TKeyItem(37, "K", "±",NULL,{SDLK_k}),       // P07, P14
        
        new TKeyItem(20, "Q", "sin", "sin-1",{SDLK_q}),       // P00, P15
        new TKeyItem(21, "W", "cos", "cos-1",{SDLK_w}),       // P01, P15
        new TKeyItem(22, "E", "tan", "tan-1",{SDLK_e}),       // P02, P15
        new TKeyItem(23, "R", "1/X", "hyp",{SDLK_r}),       // P03, P15
        new TKeyItem(24, "T", "7",NULL,{SDLK_t,SDLK_7}),       // P04, P15
        new TKeyItem(25, "Y", "8",NULL,{SDLK_y,SDLK_8}),       // P05, P15
        new TKeyItem(26, "U", "9",NULL,{SDLK_u,SDLK_9}),       // P06, P15
        new TKeyItem(27, "I", "%",NULL,{SDLK_i}),       // P07, P15
        
        new TKeyItem(28, "O", "÷", "#",{SDLK_o}),           // P00, P16
        new TKeyItem(38, "L", "x", "*",{SDLK_l}),           // P01, P16
        new TKeyItem(48, "▲", "-",NULL,{SDLK_UP}),         // P02, P16
        new TKeyItem(58, "▼", "+",NULL,{SDLK_DOWN}),     // P03, P16
        new TKeyItem(29, "P", "MC", "☎",{SDLK_p}),           // P04, P16
        new TKeyItem(39, "输入", "MR",NULL,{SDLK_RETURN}),   // P05, P16
        new TKeyItem(49, "⇟", "M-",NULL,{SDLK_SLASH}), // P06, P16
        new TKeyItem(59, "→", "M+",NULL,{SDLK_RIGHT}),   // P07, P16
        
        NULL,       // P00, P17
        NULL,       // P01, P17
        new TKeyItem(12, "F1", NULL, "插入",{SDLK_F1}),       // P02, P17
        new TKeyItem(13, "F2", NULL, "删除",{SDLK_F2}),       // P03, P17
        new TKeyItem(14, "F3", NULL, "查找",{SDLK_F3}),       // P04, P17
        new TKeyItem(15, "F4", NULL, "修改",{SDLK_F4}),       // P05, P17
        NULL,       // P06, P17
        NULL,       // P07, P17
    };


map<int,uint> sdl_to_item;
void init_keyitems(){
      for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            //fKeyItems[y][x] = item[y][x];
            if (item[y][x] == NULL) {
                //keypadmatrix[y][x] = 2;
            } else {
                int row = item[y][x]->fRow;
                int col = item[y][x]->fColumn;
                int index = row * 10 + col;
                //MyRectButton* button = [MyRectButton buttonWithType:UIButtonTypeRoundedRect];
                //button.contentScaleFactor = 1;
                //button.layer.contentsScale = 1;
                item[y][x]->tag = y * 0x10 + x;
                for(auto e: item[y][x]->sdl_keys){
                    sdl_to_item[e]=item[y][x]->tag;
                }
            }
        }
      }
}

bool InitEverything() {
  init_keyitems();
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

uint8_t map_key(int32_t sym){
  if(sdl_to_item.find(sym)!=sdl_to_item.end()){
    return sdl_to_item[sym];
  }
  return 0;
}

void SetKey(uint8_t key_id, bool down_or_up){
    unsigned int y = key_id / 16;
    unsigned int x = key_id % 16;
    if (y < 8 && x < 8) {
        keypadmatrix[y][x] = down_or_up;
    }

}

void handle_key(signed int sym, bool key_down){
        /*if(enable_debug_key_shoot){
          printf("event <%d,%d; %llu>\n", sym,key_down,SDL_GetTicks64()%1000);
        }*/
        uint8_t value=map_key(sym);
        if(value!=0){
           SetKey(value, key_down);
        }
        switch ( sym) {
          case SDLK_BACKQUOTE:
            if(key_down==1){
              //enable_dyn_debug^= 0x1;
            }
            break;

          case SDLK_TAB:
            if(key_down==1){
                //fast_forward^= 0x1;
            }
            break;

          default :  // unsupported
            break;
        }
}

void render_call_back(){
          //Render();
        //printf("i am runninging!\n");
}

void loop_run(){
    SDL_Event event;
    map<signed int, bool> mp;
    bool loop = true;
    uint64_t start_ms = SDL_GetTicks64();
    uint64_t expected_ms = 0;

    uint64_t expected_cycle=0;

    theNekoDriver->fEmulatorThread->pre_run();

    while(loop){
          expected_cycle+=num_cycle_per_batch;
          theNekoDriver->fEmulatorThread->do_run(expected_cycle);
          while (SDL_PollEvent(&event)) {
                    if ( event.type == SDL_QUIT ) {
                            loop = false;
                    } else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                        bool key_down = (event.type == SDL_KEYDOWN);

                        //try to consolidate multiple key shoot into one
                        //not sure if necessary. But it's helpful for debug
                        mp[event.key.keysym.sym]= key_down;
                        for(auto it=mp.begin();it!=mp.end();it++){
                          handle_key(it->first,it->second);
                        }
                    }
            }

          theNekoDriver->fEmulatorThread->copy_lcd_buffer();
          Render();
          expected_ms+=ms_per_batch;
          uint64_t actual_ms= SDL_GetTicks64() - start_ms;
          //if actual is behind expected_tick too much, we only remember 300ms
          if(actual_ms >expected_ms + 300) {
            expected_ms = actual_ms-300;
          }

          // similiar strategy as above
          if(expected_ms > actual_ms + 300) {
            actual_ms = expected_ms-300;
          }

          if(actual_ms <expected_ms) {
            {SDL_Delay(expected_ms-actual_ms);}
          }
    }
    theNekoDriver->fEmulatorThread->post_run();
}
int main()
{
        InitEverything();
        theNekoDriver = new TNekoDriver();
        theNekoDriver->SetLCDBufferChangedCallback(&render_call_back);
        theNekoDriver->RunDemoBin("");
        loop_run();
        return 0;
}
