#include <stdlib.h>
#include <ncurses.h>
#include <time.h>
#include <string.h>


#define MAP_ROWS 20
#define MAP_COLS 10
#define GAME_BEGIN_Y 4

#define GRID_WIDTH 2
#define GRID_HEIGH 2

#define WALL_WIDTH 1


enum mov_dir {
    HOLD = 0,
    LEFT,
    RIGHT,
    DOWN
};
uint dir_to[4][2] = {
    {0,0},
    {-1,0},
    {1, 0},
    {0, 1}
};



enum piece_block {
    I_BLOCK = 1,
    J_BLOCK,
    L_BLOCK,
    O_BLOCK,
    S_BLOCK,
    T_BLOCK,
    Z_BLOCK
};
uint8_t grid_map[MAP_ROWS][MAP_COLS];
char x;
char y;
uint8_t blk_idx;
uint8_t ticks = 0;

uint8_t next_blk_idx;
int scores = 0;

// 16bit
int block_magic[7][4] = {
  { 240,17476, 3840, 8738},
  { 113, 550, 1136, 802},
  { 116, 1570, 368,547},
  { 51, 51, 51, 51},
  { 54, 1122, 864, 561},
  { 114, 610, 624, 562},
  { 99, 612, 1584, 306}  
};


void gen_block( void )
{
    blk_idx = next_blk_idx;
    next_blk_idx = rand() % 28;
}
void gen_pos( void )
{
    x = rand() % 7;
    y = 0;
}

void gen_new_block( )
{
    gen_block(); 
    gen_pos();
}  



uint8_t transform( void )
{
   uint8_t pos = blk_idx & 0x03;
   pos = (pos + 1) % 4;
   
   return (blk_idx & 0xfc ) | pos;
}
void draw_block_cell( int cx, int cy, uint8_t idx) {
    
    	
    uint16_t magic = block_magic[idx >> 2][idx & 0x03];

    for (int i = 0; i < 16; ++i) {
       uint8_t x0 = i & 0x03;
       uint8_t y0 = i >> 2;
       
       int nx = cx + x0;
       int ny = cy + y0;

       if ( magic & (1 << i)) {
           attron(COLOR_PAIR( (idx >> 2) + 1));
	   if (nx >= 0 && ny >= 0)
	   	mvaddch( ny, nx, '#');
	   attroff(COLOR_PAIR( (idx >> 2) + 1));
       }
    }
   refresh();
}
void draw_grid_map(uint8_t x, uint8_t y)
{
    for ( int i = 0; i < MAP_ROWS; ++i) {
        for ( int j = 0; j < MAP_COLS; ++j) {
            if ( grid_map[i][j] ) {
                attron(COLOR_PAIR( grid_map[i][j] ));
	        mvaddch(i + y , j + x  , '#');
		attroff(COLOR_PAIR( grid_map[i][j]) );
	    }

	}
    } 
    refresh();
}



int check_hit( uint8_t idx, enum mov_dir dir)
{

    uint16_t magic = block_magic[ idx >> 2][ idx & 0x03];

    uint8_t nx = x + dir_to[dir][0];
    uint8_t ny = y + dir_to[dir][1];
    for ( int i = 0; i < 16; ++i) {
        uint8_t x0 = i & 0x03;
	uint8_t y0 = i >> 2;


	uint8_t cx = nx + x0;
	uint8_t cy = ny + y0;

	if ( (1 << i) & magic ) {
            
           if ( cx >= MAP_COLS || cx < 0 ) 
		   return 1;
           if ( cy >= MAP_ROWS + GAME_BEGIN_Y || cy < 0)
	           return 1;
	   if ( cy >= GAME_BEGIN_Y && grid_map[ cy - GAME_BEGIN_Y][ cx ]  )
		   return 1;
	}
    }

    return 0;
}

int check_remove ( void )
{
   
    for ( int i = MAP_ROWS  - 1; i > -1; --i) {
	int need_remove = 1;
        for ( int j = 0; j < MAP_COLS; ++j ) {
	    if ( !grid_map[i][j]) {
	        need_remove = 0;
		break;
	    }
	}
        if ( need_remove )
		return 1;
    }

    return 0;

}
int remove_line( int *line_cnt)
{
    int lowest_line = -1;
    for ( int i = MAP_ROWS - 1; i > -1; --i) {
        int need_rm = 1;
	for ( int j = 0; j < MAP_COLS; ++j) {
            if ( !grid_map[i][j]) {
	        need_rm = 0;
		break;
	    }
	}


	if ( need_rm ) {
	    if ( -1 == lowest_line)
		    lowest_line = i;
            ++*line_cnt;
	    for (int k = 0; k < MAP_COLS; ++k)
		    grid_map[i][k] = 0;
	}
    }

    // 1 line: 10 score
    // 2 line: 20 + 5
  scores +=  10 * (*line_cnt) + 5 * (*line_cnt - 1 );  
  return lowest_line; 
}


void grids_fall( int lowest_line, int line_ct )
{
  /*
  for (int i = lowest_line; i >= line_ct; --i) {
      for ( int j = 0; j < MAP_COLS; ++j)
	      grid_map[i][j] = grid_map[i - line_ct][j];
  }
*/
  memmove(grid_map + line_ct, grid_map, sizeof(uint8_t) * (lowest_line + 1 - line_ct) * MAP_COLS);
  memset( grid_map, 0, sizeof(int)*MAP_COLS * line_ct );

  /* 
    for ( int i = 0; i < MAP_COLS; ++i) {
        
        int mx = 23;
	while ( mx > 3 && grid_map[mx][i])
		--mx;
	int lowest_ept_grid = mx;
	while ( mx > 3 && !grid_map[mx][i])
		--mx;
	int lowest_grid = mx;

	int dis = lowest_ept_grid - lowest_grid;

	while ( lowest_grid > 3) {
            grid_map[ lowest_grid + dis][i] = grid_map[ lowest_grid][i];
	    grid_map[ lowest_grid][i] = 0;
	    --lowest_grid;
	}

    }
*/
}

void draw_current_block( void )
{

    // x_p = MAP_COLS + 3 
    // y_p = 2
    /*
    uint16_t magic = block_magic[ blk_idx >> 2][ blk_idx & 0x03];

    for (int i = 0; i < 16; ++i ) {
        uint8_t x0 = i & 0x03;
        uint8_t y0 = i >> 2;

	if ( (1 << i) & magic ) {
            attron( COLOR_PAIR((blk_idx >> 2) + 1));
            mvaddch( 2 + y0, MAP_COLS + 4 + x0, '$');
	    attroff( COLOR_PAIR( (blk_idx >> 2) + 1));
	}
    }*/
   
   mvprintw( 6, MAP_COLS + 4, "blk:%2d", blk_idx );
   mvprintw( 7, MAP_COLS + 4, "%2d,%2d", x, y);
   mvprintw(14, MAP_COLS + 4, "scores: %d", scores);
   mvprintw(17, MAP_COLS + 4, "w for transform");
   mvprintw(18, MAP_COLS + 4, "asd for dir");
   mvprintw(19, MAP_COLS + 4, "p for quik put");
   mvprintw(20, MAP_COLS + 4, "q for quit");
   draw_block_cell( MAP_COLS + 4, 2, blk_idx);
   draw_block_cell( MAP_COLS + 4, 10, next_blk_idx);
}
  
void draw_status_bar( void )
{
    /*
   draw_block_cell( MAP_ROWS + 4, 2, blk_idx);
   
   mvprintw( 6, MAP_COLS + 4, "%2d,%2d", blk_idx >> 2, blk_idx & 0x03 );
   mvprintw( 7, MAP_COLS + 4, "%2d,%2d", x, y);

   draw_block_cell( MAP_ROWS + 4, 9, next_blk_idx);
  */
    draw_current_block();
}
void do_remove( void )
{

     int rm_cnt = 0;
     if ( check_remove() )
     {
        
	int lowest_rm_line = remove_line( &rm_cnt );

        draw_frame();	
        grids_fall( lowest_rm_line, rm_cnt );
        draw_frame();
     }



}
void clear_map_area( void )
{
      for ( int j = 0;j < MAP_ROWS + 1; ++j)
	      mvhline(j, 0, ' ', MAP_COLS + 2);
}


void draw_block ( void )
{

   draw_block_cell( x + 1, y - GAME_BEGIN_Y , blk_idx );
   refresh();
}


void place_block( void )
{
    uint16_t magic = block_magic[ blk_idx >> 2 ][ blk_idx & 0x03];

    for ( int i = 0; i < 16; ++i ) {
        uint8_t x0 = i & 0x03;
	uint8_t y0 = i >> 2;

	if ( (1 << i) & magic ) {
            grid_map [ y+y0 - GAME_BEGIN_Y ] [ x+x0 ] = (blk_idx >> 2) + 1;
	}
    }

}
void init_ncurse_color( void )
{
    start_color( );
    for (int i = 1; i < 8; ++i ) {
        init_pair(i, i, i);
    }
}

void draw_grid( int x, int y, int color_pair_id )
{ 
 
    
}
void init_ncurse_settings( void )
{
    initscr();
    init_ncurse_color();
    noecho( );
    timeout(0);
    curs_set(0);
    resizeterm(44, 30);

}
void init_rand_generator( void )
{
    srand( time(NULL) );
    next_blk_idx = rand() % 28; 
}

int check_end( void )
{
    uint16_t magic = block_magic[ blk_idx >> 2][blk_idx & 0x03];


    for ( int i = 0; i < 16; ++i ) {
        uint8_t x0 = i & 0x03;
	uint8_t y0 = i >> 2;
       
	uint8_t nx = x + x0;
	uint8_t ny = y + y0;
	if ( (1<<i) & magic ) {
            if ( ny < GAME_BEGIN_Y)
		    return 1;
	}

    }
    return 0;
}


int do_tick( void )
{
   
    if ( ++ticks > 30) {
       ticks = 0;
       if ( check_hit(blk_idx, DOWN) ) {
             if (check_end( ))
		    return 1;
	     place_block();
	     do_remove();
	     gen_new_block(); 
	     draw_frame();
             return 0;
       }
       ++y;
    }
    return 0;
}
int process_input( int ch )
{
    
    switch ( ch )
    {
	    case 'a':
	    case 'A':
		    if ( !check_hit(blk_idx, LEFT) ) {
		        --x;
		    }
		    break;
	    case 'd':
            case 'D':
		    if ( !check_hit(blk_idx, RIGHT) ) {
                       ++x;
		    }
		    break;
	    case 's':
	    case 'S':
                    if ( !check_hit(blk_idx, DOWN ) ) {
                       ++y;
		       break;
		    }
		    if ( check_end( ) ) {
		      return 1;
		    }
		    place_block();
		    do_remove();
		    gen_new_block();
		    break;
	    case 'w':
	    case 'W':
		    if ( !check_hit(transform(), HOLD)) {
		        blk_idx = transform();
		    }
		    break;
	    case 'p':
            case 'P':
		    while ( !check_hit(blk_idx, DOWN)) {
		        ++y;
		    }
		    if ( check_end() )
			return 1;
		    place_block();
		    do_remove();
		    gen_new_block();
		    break;
            case 'q':
            case 'Q':
		    return 1;
		    break;
            default:
		    break;
    }

    return 0;
}
void draw_wall( void )
{

   // 0 col and MAP_COLS + 1 col
   // MAP_ROWS row is wall

   attron(COLOR_PAIR( 3 ));	
   mvvline( 0, 0, ' ', MAP_ROWS );
   mvvline( 0, MAP_COLS + 1, ' ', MAP_ROWS);
   mvhline( MAP_ROWS, 0, ' ',MAP_COLS + 2);
   attroff( COLOR_PAIR(3));

   /*
   for ( int i = 0; i < MAP_ROWS + 1; ++i) {
      for ( int j = 0; j < MAP_COLS + 2; ++j) {
          if ( i == 0 && (j == 0 || j == MAP_COLS + 1)) {
	      attron( COLOR_PAIR(2));
              mvaddch(i, j, '#');
	      attroff( COLOR_PAIR(2));
	  }

	  if ( i == MAP_ROWS || j == 0 || j == MAP_COLS + 1){
              attron( COLOR_PAIR(2) );
              mvaddch(i, j, '#');
	      attroff( COLOR_PAIR(2));
	  }
      }
   }
*/
    refresh();
}
void draw_frame( void )
{
   clear();
   draw_wall( );
   draw_grid_map( 1, 0);
   draw_block( );
   draw_status_bar(); 
}
void test_all_blk_show ( void )
{
    int cx = 0;
    int cy = 0;

    for ( int i = 0; i < 7; ++i,cy += 6) {
         cx = 0;
    	 for ( int j = 0; j < 4; ++j, cx += 6) {
             
	     draw_block_cell( cx, cy, i * 4 + j);
	}
    }

}


int main( int argc, char *argv[])
{
    
    init_ncurse_settings( );
    init_rand_generator( );
    gen_new_block(); 
   

    int ch;
    int is_end = 0;
    while ( !do_tick() )
    {
	usleep( 10000 );
	int ch = getch();
        if ( process_input( ch ) )
		break;
      
        draw_frame();	
    }


    endwin();

    return 0;
}
