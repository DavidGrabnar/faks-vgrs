/*
 * space_invaders.c
 *
 *  Created on: 31 Jan 2022
 *      Author: GrabPC
 */

#include <stdlib.h>
#include "space_invaders.h"
#include "assets.h"


struct si_game * si_init(Screen * screen) {

	uint8_t ** bitmaps_player = (uint8_t **) calloc(1, sizeof(uint8_t *));
	uint8_t * bitmap_player = (uint8_t *) malloc(sizeof(SI_PLAYER_BITMAP));
	memcpy(bitmap_player, SI_PLAYER_BITMAP, sizeof(SI_PLAYER_BITMAP));
	bitmaps_player[0] = bitmap_player;

	struct si_sprite * si_player_sprite = (struct si_sprite *) malloc(sizeof(struct si_sprite));

	si_player_sprite->width = SI_PLAYER_WIDTH;
	si_player_sprite->length = SI_PLAYER_LENGTH;
	si_player_sprite->scale = SI_PLAYER_SCALE;
	si_player_sprite->count = SI_PLAYER_BITMAP_COUNT;
	si_player_sprite->bitmaps = bitmaps_player;
	si_player_sprite->index = 0;

	struct si_player * si_player = (struct si_player *) malloc(sizeof(struct si_player));

	struct si_movement * si_movement_player = (struct si_movement *) malloc(sizeof(struct si_movement));
	si_movement_player->direction = SI_DIRECTION_LEFT;
	si_movement_player->offset = 0;
	si_movement_player->step = 20;

	si_player->sprite = si_player_sprite;
	si_player->movement = si_movement_player;

	uint8_t ** bitmaps_enemy1 = (uint8_t **) calloc(2, sizeof(uint8_t *));
	uint8_t * bitmap_enemy1_1 = (uint8_t *) malloc(sizeof(SI_ENEMY1_BITMAP1));
	memcpy(bitmap_enemy1_1, SI_ENEMY1_BITMAP1, sizeof(SI_ENEMY1_BITMAP1));
	bitmaps_enemy1[0] = bitmap_enemy1_1;
	uint8_t * bitmap_enemy1_2 = (uint8_t *) malloc(sizeof(SI_ENEMY1_BITMAP2));
	memcpy(bitmap_enemy1_2, SI_ENEMY1_BITMAP2, sizeof(SI_ENEMY1_BITMAP2));
	bitmaps_enemy1[1] = bitmap_enemy1_2;

	struct si_sprite * si_enemy_sprite1 = (struct si_sprite *) malloc(sizeof(struct si_sprite));

	si_enemy_sprite1->width = SI_ENEMY1_WIDTH;
	si_enemy_sprite1->length = SI_ENEMY1_LENGTH;
	si_enemy_sprite1->scale = SI_ENEMY1_SCALE;
	si_enemy_sprite1->count = SI_ENEMY1_BITMAP_COUNT;
	si_enemy_sprite1->bitmaps = bitmaps_enemy1;
	si_enemy_sprite1->index = 0;

	struct si_enemy * si_enemy1 = (struct si_enemy *) malloc(sizeof(struct si_enemy));

	si_enemy1->sprite = si_enemy_sprite1;

	struct si_enemy_group * si_enemy_groups = (struct si_enemy_group *) calloc(2, sizeof(struct si_enemy_group));

	struct si_movement * si_movement1 = (struct si_movement *) malloc(sizeof(struct si_movement));
	si_movement1->direction = SI_DIRECTION_LEFT;
	si_movement1->offset = 0;
	si_movement1->step = 20;

	si_enemy_groups[0].enemy = si_enemy1;
	si_enemy_groups[0].count = 10;
	si_enemy_groups[0].formation_width = screen->width * 0.6;
	si_enemy_groups[0].full_width = screen->width * 0.8;
	si_enemy_groups[0].movement = si_movement1;

	uint8_t ** bitmaps_enemy2 = (uint8_t **) calloc(2, sizeof(uint8_t *));
	uint8_t * bitmap_enemy2_1 = (uint8_t *) malloc(sizeof(SI_ENEMY2_BITMAP1));
	memcpy(bitmap_enemy2_1, SI_ENEMY2_BITMAP1, sizeof(SI_ENEMY2_BITMAP1));
	bitmaps_enemy2[0] = bitmap_enemy2_1;
	uint8_t * bitmap_enemy2_2 = (uint8_t *) malloc(sizeof(SI_ENEMY2_BITMAP2));
	memcpy(bitmap_enemy2_2, SI_ENEMY2_BITMAP2, sizeof(SI_ENEMY2_BITMAP2));
	bitmaps_enemy2[1] = bitmap_enemy2_2;

	struct si_sprite * si_enemy_sprite2 = (struct si_sprite *) malloc(sizeof(struct si_sprite));

	si_enemy_sprite2->width = SI_ENEMY2_WIDTH;
	si_enemy_sprite2->length = SI_ENEMY2_LENGTH;
	si_enemy_sprite2->scale = SI_ENEMY2_SCALE;
	si_enemy_sprite2->count = SI_ENEMY2_BITMAP_COUNT;
	si_enemy_sprite2->bitmaps = bitmaps_enemy2;
	si_enemy_sprite2->index = 0;

	struct si_enemy * si_enemy2 = (struct si_enemy *) malloc(sizeof(struct si_enemy));

	si_enemy2->sprite = si_enemy_sprite2;

	struct si_movement * si_movement2 = (struct si_movement *) malloc(sizeof(struct si_movement));
	si_movement2->direction = SI_DIRECTION_LEFT;
	si_movement2->offset = 0;
	si_movement2->step = 10;

	si_enemy_groups[1].enemy = si_enemy2;
	si_enemy_groups[1].count = 5;
	si_enemy_groups[1].formation_width = screen->width * 0.6;
	si_enemy_groups[1].full_width = screen->width * 0.8;
	si_enemy_groups[1].movement = si_movement2;

	struct si_level * si_level1 = (struct si_level *) malloc(sizeof(struct si_level));

	si_level1->groups = si_enemy_groups;
	si_level1->group_count = 2;

	struct si_game * game = (struct si_game *) malloc(sizeof(struct si_game));

	game->levels = si_level1;
	game->player = si_player;
	game->level_count = 1;
	game->header_height = 100;
	game->tick_duration = 100;

	return game;
}

void si_update(Screen * screen, struct si_game * game) {
	// update level
	struct si_level *curr_level = &game->levels[0];

	for (int enemy_group_index = 0; enemy_group_index < curr_level->group_count; enemy_group_index++) {
		struct si_enemy_group *curr_group = &curr_level->groups[enemy_group_index];

		int max_offset = (curr_group->full_width - curr_group->formation_width) / 2;
		si_update_movement(curr_group->movement, max_offset);
		si_update_sprite(curr_group->enemy->sprite);
	}

	si_update_movement(game->player->movement, (screen->width - game->player->sprite->width * game->player->sprite->scale) / 2);
	si_update_sprite(game->player->sprite);
}

void si_update_movement(struct si_movement * movement, int max_offset)
{
	int curr_offset = movement->offset;
	int curr_direction = movement->direction;

	int curr_step = movement->step * curr_direction;

	int next_offset = curr_offset + curr_step;
	int next_direction = curr_direction;
	if (abs(next_offset) > max_offset) {
		next_offset = (next_offset / abs(next_offset)) * (max_offset + (next_offset % max_offset));
		next_direction *= -1;
	}

	movement->offset = next_offset;
	movement->direction = next_direction;
}

void si_update_sprite(struct si_sprite * sprite)
{
	if (sprite->count == 1) return;

	sprite->index = (sprite->index + 1) % sprite->count;
}

void si_render(Screen * screen, struct si_game * game)
{
	//render game
	BSP_LCD_Clear(LCD_COLOR_BLACK);

	BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
	BSP_LCD_FillRect(0, 0, screen->width, game->header_height);

	//render level
	struct si_level *curr_level = &game->levels[0];
	int group_pos_y = game->header_height;
	// render enemy groups
	for (int enemy_group_index = 0; enemy_group_index < curr_level->group_count; enemy_group_index++) {
		struct si_enemy_group * curr_group = &curr_level->groups[enemy_group_index];
		struct si_enemy * curr_enemy = curr_group->enemy;
		struct si_sprite * curr_sprite = curr_enemy->sprite;
		int height = curr_sprite->length / curr_sprite->width;
		int enemy_per_row = curr_group->formation_width / (curr_sprite->width * curr_sprite->scale); // floor
		int enemy_rows = (curr_group->count + (enemy_per_row - 1)) / enemy_per_row; // ceil
		int enemy_last_row = curr_group->count % enemy_per_row;

		for (int row_index = 0; row_index < enemy_rows; row_index++) {
			int per_row = row_index == enemy_rows - 1 ? enemy_last_row : enemy_per_row;

			int group_space_left_width = curr_group->formation_width - (curr_sprite->width * curr_sprite->scale) * per_row;
			int enemy_pos_x = (screen->width - curr_group->formation_width) / 2 + group_space_left_width / 2 + curr_group->movement->offset;
			// render enemy
			for (int cell_index = 0; cell_index < per_row; cell_index++) {
				si_render_sprite(curr_sprite, enemy_pos_x, group_pos_y);

				enemy_pos_x += curr_sprite->width * curr_sprite->scale;
			}
			group_pos_y += (height * curr_sprite->scale);
		}
	}

	// render player
	struct si_player * player = game->player;
	struct si_sprite * sprite = player->sprite;

	int space_left_width = screen->width - (sprite->width * sprite->scale);
	int pos_x = space_left_width / 2 + player->movement->offset;
	si_render_sprite(sprite, pos_x, screen->height - (sprite->length / sprite->width) * sprite->scale);
}

void si_render_sprite(struct si_sprite * sprite, int x, int y) {
	uint8_t * bitmap = sprite->bitmaps[sprite->index];
	int height = sprite->length / sprite->width;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < sprite->width; j++) {
			int bit_index = i * sprite->width + j;
			int byte_index = bit_index / 8;
			int bit_offset = bit_index % 8;

			uint32_t color = bitmap[byte_index] & (1 << (7 - bit_offset))
				? LCD_COLOR_WHITE
				: LCD_COLOR_BLACK;

			BSP_LCD_SetTextColor(color);
			BSP_LCD_FillRect(x + j * sprite->scale, y + i * sprite->scale, sprite->scale, sprite->scale);
		}
	}
}
