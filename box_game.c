#include "box_game.h"
#include "3595_LCD.h"

/*
Program flow:
  -Initialize
  -Spawn piece
    -load from reference
    -calculate y_loc
    -draw piece
  -Wait for game input
*/

/**BOX_location[] Array*********************
* Used to track where boxes are in the     *
* playing area.  One byte per column with  *
* byte0 as the left column.  One bit per   *
* row with the LSB at the top.  This makes *
* for a 12x8 playing area                  *
*******************************************/

unsigned char BOX_location[12] = {
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000
};

/*********************************************
* BOX_piece[] is a 4x4 represntation of      *
* the currently selected playing piece.      *
* When a piece spawns its shape is           *
* written to this array in four nibbles      *
* Byte0,LSN; Byte0,MSN; Byte1,LSN; Byte1,MSN *
* The LSB of each nibble is the top of       *
* the display area.                          *
*********************************************/

unsigned char BOX_piece[] = { 0b00000000, 0b00000000 };

static const char PROGMEM BOX_reference[] = {
  //  _|_ (T)
    0b11001000, 0b00001000,
    0b11100000, 0b00000100,
    0b11000100, 0b00000100,
    0b11100100, 0b00000000,
  
  //  _|-- (S)
    0b11001000, 0b00000100,
    0b11000110, 0b00000000,
    0b11001000, 0b00000100,
    0b11000110, 0b00000000,

  // --|_ (Z)
    0b11000100, 0b00001000,
    0b01101100, 0b00000000,
    0b11000100, 0b00001000,
    0b01101100, 0b00000000,

  // |_ (L)
    0b01001100, 0b00000100,
    0b00100000, 0b00001110,
    0b10001000, 0b00001100,
    0b11100000, 0b00001000,

  // _| (backward L)
    0b01000100, 0b00001100,
    0b10000000, 0b00001110,
    0b10001100, 0b00001000,
    0b11100000, 0b00000010,

  // |=| (box)
    0b11001100, 0b00000000,
    0b11001100, 0b00000000,
    0b11001100, 0b00000000,
    0b11001100, 0b00000000,

  // ---- (line)  
    0b01000100, 0b01000100,
    0b11110000, 0b00000000,
    0b01000100, 0b01000100
    0b11110000, 0b00000000,
};

//Variables
unsigned char x_loc, y_loc;     //Bottom left index of each piece
unsigned char cur_piece = 0;	//Index for BOX_reference
unsigned char rotate = 0;	//Index for piece rotation

//Functions
void BOX_store_loc(void)		//Stores current x_loc & y_loc to array
{
  for (unsigned char i=0; i<4; i++)  //Step through each of 4 columns
  {
    if ((x_loc+i) > BOX_board_right) return;  //Prevent invalid x_loc
    for (unsigned char j=0; j<4; j++) //Step through each of 4 rows
    {
    //prevent invalid indicies from being written
      if ((y_loc-j) >= BOX_board_top)
      {
	if (BOX_piece[i/2] & 1<<((4*(i%2))+(3-j)))
	{
	  BOX_location[x_loc+i] |= 1<<(y_loc-j);
	}
      }
    }
  }
}

void BOX_clear_loc(void)		//Stores current x_loc & y_loc to array
{
  for (unsigned char i=0; i<4; i++)  //Step through each of 4 columns
  {
    if ((x_loc+i) > BOX_board_right) return;  //Prevent invalid x_loc
    for (unsigned char j=0; j<4; j++) //Step through each of 4 rows
    {
    //prevent invalid indicies from being written
      if ((y_loc-j) >= BOX_board_top)
      {
	if (BOX_piece[i/2] & 1<<((4*(i%2))+(3-j)))
	{
	  BOX_location[x_loc+i] &= ~(1<<(y_loc-j));
	}
      }
    }
  }
}


void BOX_load_reference(unsigned char piece, unsigned char rotation)
{
//pgm_read_byte((char *)((int)font5x8 + (5 * letter) + i));
  BOX_piece[0] = pgm_read_byte((char *)(BOX_reference + (piece*8) + (rotation*2)));
  BOX_piece[1] = pgm_read_byte((char *)(BOX_reference + (piece*8) + (rotation*2) + 1));
}

void BOX_rotate(unsigned char direction)
{
  //TODO: Check if we are going to hit something when we rotate
  //TODO: Check if we will go off the screen when we rotate
  BOX_clear_loc();
  BOX_clear_piece();
  if (++rotate > 3) rotate = 0;
  BOX_load_reference(cur_piece, rotate);
  BOX_write_piece();
  BOX_store_loc();
}

void BOX_draw(unsigned char X, unsigned char Y)
{
  LCD_Out(0x2A, 1); //Set Column location
  LCD_Out(X*8, 0);
  LCD_Out((X*8)+7, 0);
  LCD_Out(0x2B, 1); //Set Row location
  LCD_Out(Y*8, 0);
  LCD_Out((Y*8)+7, 0);
  LCD_Out(0x2C, 1); //Write Data
  for (unsigned char i=0; i<64; i++) LCD_Out(red,0);
}
void BOX_erase(unsigned char X, unsigned char Y)
{
  LCD_Out(0x2A, 1); //Set Column location
  LCD_Out(X*8, 0);
  LCD_Out((X*8)+7, 0);
  LCD_Out(0x2B, 1); //Set Row location
  LCD_Out(Y*8, 0);
  LCD_Out((Y*8)+7, 0);
  LCD_Out(0x2C, 1); //Write Data
  for (unsigned char i=0; i<64; i++) LCD_Out(white,0);
}

void BOX_write_piece(void)  //Writes piece to display
{
  for (unsigned char i=0; i<4; i++)  //Step through each of 4 columns
  {
    for (unsigned char j=0; j<4; j++) //Step through each of 4 rows
    {
    //prevent invalid indicies from being written
      if ((y_loc-j) >= BOX_board_top)
      {
	if (BOX_piece[i/2] & 1<<((4*(i%2))+(3-j)))
	{
	  BOX_draw(x_loc+i, y_loc-j);
	}
      }
    }
  }
}

void BOX_clear_piece(void)  //Clears piece from display
{
  for (unsigned char i=0; i<4; i++)  //Step through each of 4 columns
  {
    for (unsigned char j=0; j<4; j++) //Step through each of 4 rows
    {
    //prevent invalid indicies from being written
      if ((y_loc-j) >= BOX_board_top)
      {
	if (BOX_piece[i/2] & 1<<((4*(i%2))+(3-j)))
	{
	  BOX_erase(x_loc+i, y_loc-j);
	}
      }
    }
  }
}

void BOX_spawn(void)
{
  x_loc = 4;
  y_loc = 0;
  rotate = 0;

  BOX_load_reference(cur_piece, rotate);  //load from reference
  
  //calculate y_loc
  for (unsigned char i=0; i<3; i++)
  {
    if((BOX_piece[0] | BOX_piece[1]) & 0b00010001<<i) y_loc++; //There is a box in the row, make sure we see it
  }

  BOX_store_loc(); //Store new location
  BOX_write_piece(); //draw piece
}

void BOX_up(void)
{
  BOX_clear_loc();
  BOX_clear_piece();

  if (++cur_piece > 6) cur_piece = 0;
  x_loc = 4;
  y_loc = 0;

  BOX_spawn();
}

void BOX_dn(void)
{
  if (((y_loc == BOX_board_bottom) && ((BOX_piece[0] | BOX_piece[1]) & 0x88)) ||
	((y_loc-1 == BOX_board_bottom) && ((BOX_piece[0] | BOX_piece[1]) & 0x44))) return; //Do nothing if we're at the bottom already
  //if (BOX_location[x_loc] & 1<<(y_loc+1)) return; //Do nothing if there is a box below us
  BOX_clear_loc();
  BOX_clear_piece();
  ++y_loc;
  BOX_write_piece();
  BOX_store_loc();
}

void BOX_lt(void)
{
  if (((x_loc == BOX_board_left) && (BOX_piece[0] & 0x0F)) || (x_loc == 255)) return; //Do nothing if we're at the left edge already
  //if (BOX_location[x_loc-1] & 1<<y_loc) return; //Do nothing if there is a box beside us
  BOX_clear_loc();
  BOX_clear_piece();
  x_loc--;
  BOX_write_piece();
  BOX_store_loc();
}

void BOX_rt(void)
{
  if (((x_loc+3 == BOX_board_right) && (BOX_piece[1] & 0xF0)) ||
      ((x_loc+2 == BOX_board_right) && (BOX_piece[1] & 0x0F)) || 
      (x_loc+1 == BOX_board_right)) return; //Do nothing if we're at the right edge already
  //if (BOX_location[x_loc+1] & 1<<y_loc) return; //Do nothing if there is a box beside us
  BOX_clear_loc();
  BOX_clear_piece();
  ++x_loc;
  BOX_write_piece();
  BOX_store_loc();
}