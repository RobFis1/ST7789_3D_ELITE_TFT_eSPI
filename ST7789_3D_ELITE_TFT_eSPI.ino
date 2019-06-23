/*
 * Based on an idea of https://www.reddit.com/r/EliteDangerous/comments/bjdg54/old_school_elite_ship_viewer/
 * 
 * Code base on http://colinord.blogspot.com/2015/01/arduino-oled-module-with-3d-demo.html
 */
#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include "Free_Fonts.h"

#include "ships.h"

TFT_eSPI tft = TFT_eSPI();

TFT_eSprite img = TFT_eSprite(&tft);  // Create Sprite object "img" with pointer to "tft" object
// the pointer is used by pushSprite() to push it onto the TFT

#define BITS_PER_PIXEL 1              // How many bits per pixel in Sprite

//float tx, nx, p;
//float ty, ny, py;
float rot, rotx, roty, rotz, rotxx, rotyy, rotzz, rotxxx, rotyyy, rotzzz;
int i; //0 to 360
int fl, scale; //focal length
int wireframe[32][3];

int ship_vertices_cnt, ship_faces_cnt;
int ship_vertices[32][3];
int ship_faces[32][9];


int originx = 120;
int originy = 70;

int front_depth = 20;
int back_depth = -20;

float vector;

int fd = 0; //0=orthographic

float scalefactor = 0;

void setup(void)
{
  Serial.begin(115200);
  tft.init();   // initialize a ST7789 chip, 240x240 pixels
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 15, 2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK); tft.setTextFont(1);
  tft.setFreeFont(FF6);
  tft.println("Commander JAMESON");
  tft.setCursor(0, 40, 2);
  tft.println("Rank: E L I T E");
  //tft.println("Cobra MK I");
}


void draw_wireframe_ship(void)
{
  int face, f_line, wf_f_1, wf_f_2;

  img.setColorDepth(8);         // Set colour depth first
  img.createSprite(240, 140);                  // then create the sprite
  img.fillSprite(TFT_BLACK);

  for ( face = 0; face < ship_faces_cnt; face++) {
    vector = 0;
    for ( f_line = 1; f_line < ship_faces[face][0]; f_line++) {
      wf_f_1 = ship_faces[face][f_line];
      wf_f_2 = ship_faces[face][f_line + 1];
      vector += wireframe[wf_f_1][0] * wireframe[wf_f_2][1] - wireframe[wf_f_1][1] * wireframe[wf_f_2][0];
    };
    wf_f_1 = ship_faces[face][f_line];
    wf_f_2 = ship_faces[face][1];
    vector += wireframe[wf_f_1][0] * wireframe[wf_f_2][1] - wireframe[wf_f_1][1] * wireframe[wf_f_2][0];

    //Serial.printf("Face %02i ", face);
    //Serial.println(vector);
    if ( vector >= 0 ) {
      for ( f_line = 1; f_line < ship_faces[face][0]; f_line++) {
        wf_f_1 = ship_faces[face][f_line];
        wf_f_2 = ship_faces[face][f_line + 1];
        img.drawLine(wireframe[wf_f_1][0], wireframe[wf_f_1][1], wireframe[wf_f_2][0], wireframe[wf_f_2][1], TFT_WHITE);
      }
      wf_f_1 = ship_faces[face][f_line];
      wf_f_2 = ship_faces[face][1];
      img.drawLine(wireframe[wf_f_1][0], wireframe[wf_f_1][1], wireframe[wf_f_2][0], wireframe[wf_f_2][1], TFT_WHITE);
    }
  }
  img.setBitmapColor(TFT_WHITE, TFT_BLACK);
  // Push sprite to TFT screen CGRAM at coordinate x,y (top left corner)
  // Specify what colour is to be treated as transparent (black in this example)
  img.pushSprite(0, 50);

  // Delete Sprite to free memory, creating and deleting takes very little time.
  img.deleteSprite();

}

void rotate_ship(void) {
  scalefactor = 0;
  for (int cnt = 0; cnt < 3; cnt ++) {
    for (int angle = 0; angle <= 360; angle = angle + 3) {
      for (int i = 0; i < ship_vertices_cnt; i++) {
        rot = angle * 0.0174532; //0.0174532 = one degree
        //rotateY
        rotz = ship_vertices[i][2] / scale * sin(rot) - ship_vertices[i][0] / scale * cos(rot);
        rotx = ship_vertices[i][2] / scale * cos(rot) + ship_vertices[i][0] / scale * sin(rot);
        roty = ship_vertices[i][1] / scale;
        //rotateX
        rotyy = roty * sin(rot) - rotz * cos(rot);
        rotzz = roty * cos(rot) + rotz * sin(rot);
        rotxx = rotx;
        //rotateZ
        rotxxx = rotxx * sin(rot) - rotyy * cos(rot);
        rotyyy = rotxx * cos(rot) + rotyy * sin(rot);
        rotzzz = rotzz;

        //orthographic projection
        //rotxxx = rotxxx + originx;
        //rotyyy = rotyyy + originy;
        rotxxx = rotxxx * scalefactor + originx;
        rotyyy = rotyyy * scalefactor + originy;

        //store new vertices values for wireframe drawing
        wireframe[i][0] = rotxxx;
        wireframe[i][1] = rotyyy;
        wireframe[i][2] = rotzzz;

        //draw_vertices();
      }
      //tft.fillScreen(TFT_BLACK);
      draw_wireframe_ship();
      if (scalefactor < 1) scalefactor = scalefactor + 0.02;
      delay(10);
    }
  }
}

void loop(void)
{
  int i, j;

  tft.fillRect(0, 200, 239, 40, TFT_BLACK);
  tft.setCursor(10, 234, 2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);  tft.setTextSize(1);
  tft.println("Cobra MK I             ");
  memcpy(ship_vertices, cobra_vertices, sizeof(cobra_vertices));
  ship_vertices_cnt = cobra_vertices_cnt;
  scale = cobra_scale;
  memcpy(ship_faces, cobra_faces, sizeof(cobra_faces));
  ship_faces_cnt = cobra_faces_cnt;
  rotate_ship();

  tft.fillRect(0, 200, 239, 40, TFT_BLACK);
  tft.setCursor(10, 234, 2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);  tft.setTextSize(1);
  tft.println("Adder             ");
  memcpy(ship_vertices, adder_vertices, sizeof(adder_vertices));
  ship_vertices_cnt = adder_vertices_cnt;
  scale = adder_scale;
  memcpy(ship_faces, adder_faces, sizeof(adder_faces));
  ship_faces_cnt = adder_faces_cnt;
  rotate_ship();

  tft.fillRect(0, 200, 239, 40, TFT_BLACK);
  tft.setCursor(10, 234, 2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);  tft.setTextSize(1);
  tft.println("ASP             ");
  memcpy(ship_vertices, asp_vertices, sizeof(asp_vertices));
  ship_vertices_cnt = asp_vertices_cnt;
  scale = asp_scale;
  memcpy(ship_faces, asp_faces, sizeof(asp_faces));
  ship_faces_cnt = asp_faces_cnt;
  rotate_ship();

  tft.fillRect(0, 200, 239, 40, TFT_BLACK);
  tft.setCursor(10, 234, 2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);  tft.setTextSize(1);
  tft.println("Anaconda             ");
  memcpy(ship_vertices, anaconda_vertices, sizeof(anaconda_vertices));
  ship_vertices_cnt = anaconda_vertices_cnt;
  scale = anaconda_scale;
  memcpy(ship_faces, anaconda_faces, sizeof(anaconda_faces));
  ship_faces_cnt = anaconda_faces_cnt;
  rotate_ship();

  tft.fillRect(0, 200, 239, 40, TFT_BLACK);
  tft.setCursor(10, 234, 2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);  tft.setTextSize(1);
  tft.println("Viper      ");
  memcpy(ship_vertices, viper_vertices, sizeof(viper_vertices));
  ship_vertices_cnt = viper_vertices_cnt;
  scale = viper_scale;
  memcpy(ship_faces, viper_faces, sizeof(viper_faces));
  ship_faces_cnt = viper_faces_cnt;
  rotate_ship();

  tft.fillRect(0, 200, 239, 40, TFT_BLACK);
  tft.setCursor(10, 234, 2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);  tft.setTextSize(1);
  tft.println("Sidewinder      ");
  memcpy(ship_vertices, sidewinder_vertices, sizeof(sidewinder_vertices));
  ship_vertices_cnt = sidewinder_vertices_cnt;
  scale = sidewinder_scale;
  memcpy(ship_faces, sidewinder_faces, sizeof(sidewinder_faces));
  ship_faces_cnt = sidewinder_faces_cnt;
  rotate_ship();

  tft.fillRect(0, 200, 239, 40, TFT_BLACK);
  tft.setCursor(10, 234, 2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);  tft.setTextSize(1);
  tft.println("Coriolis Station");
  memcpy(ship_vertices, coriolis_vertices, sizeof(coriolis_vertices));
  ship_vertices_cnt = coriolis_vertices_cnt;
  scale = coriolis_scale;
  memcpy(ship_faces, coriolis_faces, sizeof(coriolis_faces));
  ship_faces_cnt = coriolis_faces_cnt;
  rotate_ship();

  tft.fillRect(0, 200, 239, 40, TFT_BLACK);
  tft.setCursor(10, 234, 2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);  tft.setTextSize(1);
  tft.println("Dodo Station");
  memcpy(ship_vertices, dodo_vertices, sizeof(dodo_vertices));
  ship_vertices_cnt = dodo_vertices_cnt;
  scale = dodo_scale;
  memcpy(ship_faces, dodo_faces, sizeof(dodo_faces));
  ship_faces_cnt = dodo_faces_cnt;
  rotate_ship();

  tft.fillRect(0, 200, 239, 40, TFT_BLACK);
  tft.setCursor(10, 234, 2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);  tft.setTextSize(1);
  tft.println("Thargoid");
  memcpy(ship_vertices, thargoid_vertices, sizeof(thargoid_vertices));
  ship_vertices_cnt = thargoid_vertices_cnt;
  scale = thargoid_scale;
  memcpy(ship_faces, thargoid_faces, sizeof(thargoid_faces));
  ship_faces_cnt = thargoid_faces_cnt;
  rotate_ship();
}
