{
int playing;
int old_map_pos;
int level;
int diff;
int i;
int quit; quit = 0;
int scroll_acc;
int scroll; scroll = -1;
int max_scroll;
int speeds[9] = { 1500, 3000, 4500, 6000, 7500, 9000, 10500, 1800000, 9000000 };
int next_speed; next_speed = 0;
int next_aight; next_aight = 50;
int allow_smpl; allow_smpl = 1;
int game_over; game_over = 0;
int falling; falling = 0;
int shake; shake = 0;
int flash;
int step_count; step_count = 0;
int next_floor; next_floor = -1;
int play_again;
int tot_scroll;
int lastX; int lastY; int midX; int midY;
int numComboJumps; numComboJumps = 0;
int totComboFloors; totComboFloors = 0;
int startTime;
int endTime; endTime = 0;
int lastJumpLength; lastJumpLength = 0;

int oldUnlockedFloors;
int current_rank_id;

Tcontrol rec_ctrl; rec_ctrl = ctrl;

if (!itrcheck) {
oldUnlockedFloors = profile->best_floor / 100;
current_rank_id = get_rank_id(profile); } else { current_rank_id = 0; oldUnlockedFloors = 0;
}




int time_cheat_count; time_cheat_count = 0;

clock_t clockTimeStart; clock_t clockTimeEnd;
double clockElapsed;
double totClockTimes;


int qpc_start; int qpc_end;

double qpc_elapsed;
double totQPCTimes;

int timeTimeStart; int timeTimeEnd;
int timeElapsed;
double totTimeTimes;

int musicCounter; musicCounter = 0;
int lastMusicPos; lastMusicPos = 0;
float accMusics; accMusics = 0.0f;
int totMusics; totMusics = 0;




if (recording) {
demo->tc_posts = 0;
for (i = 0; i < 100; i++) {
demo->tc_c_data[i] = 0.0f;
demo->tc_q_data[i] = 0.0f;
demo->tc_t_data[i] = 0.0f;
demo->tc_s_data[i] = 0.0f;
demo->tc_f_data[i] = 0.0f;
}
}


log2file(" setting up play data");
fall_count = 0;
clock_angle = 0;
map.offset = 0;

fast_forward = 0;
fast_fast_forward = 0;

update_frame();
if (!itrcheck) {
draw_frame(swap_screen);

fadeIn(swap_screen, 16);
play_sound(custom.yo, 0, 0);
startGameMusic();
}

if (!itrcheck)
if (bg_beat) {
checkMusicVoiceID = play_sample(bg_beat, 0, 128, 1000, 1);
}




cycle_count = 0;

log2file(" play started");
startTime = time(NULL);




LARGE_INTEGER li;
QueryPerformanceCounter(&li);
qpc_start = li.LowPart;

QueryPerformanceFrequency(&li);
int qpc_freq;




clockTimeStart = clock();

timeTimeStart = time(NULL);


playing = TRUE;
while (playing) {

if (closeButtonClicked) return 0; {



cycle_count = 0;

logic_count++;
step_count++;
fall_count++;
time_cheat_count++;
musicCounter++;


if (!itrcheck) {
if (lastFocus != hasFocus) {
if (hasFocus) {
if (bg_beat) {
checkMusicVoiceID = play_sample(bg_beat, 0, 128, 1000, 1);
}




startGameMusic(); totMusics = 0; accMusics = 0.0f; musicCounter = 0;
} else {

if (checkMusicVoiceID >= 0) {
voice_stop(checkMusicVoiceID);
}
checkMusicVoiceID = -1;

stopGameMusic();
}
lastFocus = hasFocus;
}
}


if (!itrcheck) { if (checkMusicVoiceID >= 0) {
int vgp; vgp = voice_get_position(checkMusicVoiceID);
if (vgp < lastMusicPos) { musicCounter = 0;
}


float a = vgp / 44000.0;
float b;

if (a > 0.01) {
b = musicCounter / 50.0; accMusics += b / a;
totMusics++; } lastMusicPos = vgp;
}
}







if (recording && map.offset > 100 && !ply[player_id]->dead) {

if (time_cheat_count == 1000) {



double clockSpeed; double qpcSpeed; double timeSpeed;


clockTimeEnd = clock();
clockElapsed = clockTimeEnd - clockTimeStart; clockSpeed = (50.0 * clockElapsed) / 1000.0;
if (clockSpeed > 0.0) { totClockTimes = (1000.0 * clockSpeed) / clockSpeed / 20.0; } else { totClockTimes = -0.05;
}


QueryPerformanceFrequency(&li);
qpc_freq = li.LowPart;
QueryPerformanceCounter(&li); qpc_end = li.LowPart;


qpc_elapsed = qpc_end - qpc_start; qpcSpeed = (50.0 * qpc_elapsed / qpc_freq) / 20.0; totQPCTimes = qpcSpeed;







timeTimeEnd = time(NULL);









timeElapsed = timeTimeEnd - timeTimeStart;
timeSpeed = (50.0 * timeElapsed) / 20.0;
totTimeTimes = timeSpeed;
demo->tc_c_data[demo->tc_posts] = 0.0 + totClockTimes;
demo->tc_q_data[demo->tc_posts] = 0.0 + totQPCTimes;
demo->tc_t_data[demo->tc_posts] = 0.0 + totTimeTimes;
demo->tc_f_data[demo->tc_posts] = ply[player_id]->level;
if (totMusics != 0) {
demo->tc_s_data[demo->tc_posts] = 50.0 * accMusics / totMusics;
}
demo->tc_posts = demo->tc_posts < 98 ? demo->tc_posts + 1 : 99;










clockTimeStart = clock();

QueryPerformanceCounter(&li);
qpc_start = li.LowPart;



timeTimeStart = time(NULL); totMusics = 0; accMusics = 0.0f; time_cheat_count = 0;
}
}

















if (debug) {
if (key[KEY_1]) { if (allow_smpl) start_reward(5); }
if (key[KEY_2]) { if (allow_smpl) start_reward(7); }
if (key[KEY_3]) { if (allow_smpl) start_reward(15); }
if (key[KEY_4]) { if (allow_smpl) start_reward(25); }
if (key[KEY_5]) { if (allow_smpl) start_reward(35); }
if (key[KEY_6]) { if (allow_smpl) start_reward(50); }
if (key[KEY_7]) { if (allow_smpl) start_reward(70); }
if (key[KEY_8]) { if (allow_smpl) start_reward(100); }
if (key[KEY_9]) { if (allow_smpl) start_reward(140); }
if (key[KEY_0]) { if (allow_smpl) start_reward(200); }
allow_smpl = !(key[KEY_1] || key[KEY_2] || key[KEY_3] || key[KEY_4] || key[KEY_5] || key[KEY_6] || key[KEY_7] || key[KEY_8] || key[KEY_9] || key[KEY_0]);
}




midX = (int)ply[player_id]->x;
midY = (int)ply[player_id]->y;


handle_player_input(&ctrl);
update_player(ply[player_id]);


if (!itrcheck) {
if (ply[player_id]->rotate && ply[player_id]->in_combo && !options.flash)
create_particle(stars, (int)ply[player_id]->x, (int)ply[player_id]->y - 16);


for (i = 0; i < 512; i++) if (stars[i].intensity) update_particle(&stars[i]); } lastY = midY;





old_map_pos = map.offset; scroll_acc = 0;

if (ply[player_id]->y < 160.0) { scroll_acc = 1;

if (ply[player_id]->y < 140.0) if (ply[player_id]->y < 140.0) scroll_acc++;
if (ply[player_id]->y < 120.0) if (ply[player_id]->y < 120.0) scroll_acc++;
if (ply[player_id]->y < 100.0) if (ply[player_id]->y < 100.0) scroll_acc++;
if (ply[player_id]->y < 80.0) if (ply[player_id]->y < 80.0) scroll_acc++;
if (ply[player_id]->y < 60.0) if (ply[player_id]->y < 60.0) scroll_acc++;
if (ply[player_id]->y < 40.0) if (ply[player_id]->y < 40.0) scroll_acc += 2;
if (ply[player_id]->y < 20.0) if (ply[player_id]->y < 20.0) scroll_acc += 2;
if (ply[player_id]->y < 0.0) if (ply[player_id]->y < 0.0) scroll_acc += 3;
map.offset = old_map_pos + scroll_acc;

ply[player_id]->y += scroll_acc;
lastY = midY + scroll_acc; } tot_scroll = scroll_acc;



if (!ply[player_id]->dead) clock_angle++;

if (map.offset > 100 && !ply[player_id]->dead) {
if (scroll == -1)
scroll = start_speeds[demo->start_speed];

if (!scroll) {
if (step_count & 1) {
map.offset++;
tot_scroll++;
ply[player_id]->y += 1.0;
lastY++;
}
}
else {
map.offset += scroll;
tot_scroll += scroll;
ply[player_id]->y += scroll;
lastY += scroll;
}
}
else if (!ply[player_id]->dead) {
clock_angle = 0; fall_count = 0;
}



any13 = tot_scroll;

if (hurry_y > -100 && hurry_y < 480) hurry_y -= 2;
if (demo->speed_increase)
if (!ply[player_id]->dead && speeds[next_speed] < fall_count && scroll < 5) {
ply[player_id]->ccc[next_speed] = ply[player_id]->level;

next_speed++;
scroll++;
hurry_y = 479;
play_sound(speaker[0], 0, 0);
play_sound(sounds[4], 0, 0);
}


if (scroll == 5) {
fall_count -= 45;
if (!ply[player_id]->dead) clock_angle -= 45;
}

if (old_map_pos % 16 > map.offset % 16) {
add_floor(&map);

} else
if (tot_scroll > 15) {

add_floor(&map);
}























switch (collision_type) { case 0:
handle_player_collision_original(midX, lastY);
break; case 1:
handle_player_collision_old(midX, lastY);
break; case 2:
handle_player_collision_vector(midX, lastY);
break; case 3:
handle_player_collision_vector_2(midX, lastY);
break; case 4:
handle_player_collision_combo(midX, lastY);
break; default:

allegro_message("unknown collision type"); break;
}





if (ply[player_id]->rotate) ply[player_id]->angle += itofix(8);



if (ply[player_id]->in_combo) {
ply[player_id]->in_combo--;
if (!ply[player_id]->in_combo)
if (ply[player_id]->acc_jumps > 1) {
ply[player_id]->score += ply[player_id]->acc_level * ply[player_id]->acc_level;
int rewResult; rewResult = start_reward(ply[player_id]->acc_level);
if (recording && !is_playing_custom_game) profile->rewards[rewResult]++;
totComboFloors += ply[player_id]->acc_level;
numComboJumps++;

Tgd_combo c;
c.length = ply[player_id]->acc_level;
c.start = gdComboStart;
c.end = c.start + c.length;
add_combo(gameData, &c);

ply[player_id]->latest_combo = ply[player_id]->acc_level;
if (ply[player_id]->acc_level > ply[player_id]->best_combo)
ply[player_id]->best_combo = ply[player_id]->acc_level;
}
}




if (!ply[player_id]->status) {

level = (get_level(&map, (int)ply[player_id]->y) - 1) / 5;




diff = level - ply[player_id]->level;
if (diff != 0) {
if (diff != gdLastJumpDiff) {


jumpSequence.dist = gdLastJumpDiff;
add_jump_sequence(gameData, &jumpSequence);


jumpSequence.num = 1;
jumpSequence.start = level - diff;


} else jumpSequence.num++;


gdLastJumpDiff = diff;
}




if (level >= ply[player_id]->level) {

diff = level - ply[player_id]->level;


if (diff != lastJumpLength && diff != 0) {
for (i = 0; i < 5; i++) {


if (ply[player_id]->jc[i] > ply[player_id]->jcTop[i])
ply[player_id]->jcTop[i] = ply[player_id]->jc[i];


ply[player_id]->jc[i] = 0; } lastJumpLength = 0;
}





if (diff > 0) {
if (diff <= 5)
ply[player_id]->jc[diff - 1]++; lastJumpLength = diff;
}




if (diff > 0 && diff != 1) {
if (ply[player_id]->in_combo) {
ply[player_id]->acc_level += diff;
ply[player_id]->acc_jumps++;
ply[player_id]->in_combo = 100;
}
else {
ply[player_id]->acc_level = diff;
ply[player_id]->acc_jumps = 1;
ply[player_id]->in_combo = 100;
}
}

if (diff == 1 && ply[player_id]->in_combo)
ply[player_id]->in_combo = 1;


if (!ply[player_id]->in_combo)
gdComboStart = level;
}
else {



if (ply[player_id]->in_combo) ply[player_id]->in_combo = 1;

for (i = 0; i < 5; i++) {


if (ply[player_id]->jc[i] > ply[player_id]->jcTop[i])
ply[player_id]->jcTop[i] = ply[player_id]->jc[i];


ply[player_id]->jc[i] = 0; } lastJumpLength = 0;
}








ply[player_id]->level = level;




if (!numComboJumps && ply[player_id]->no_combo_top_floor < ply[player_id]->level)

ply[player_id]->no_combo_top_floor = gdComboStart;
}





if (ply[player_id]->y > 540.0 && !ply[player_id]->dead) {
if (itrcheck) playing = FALSE;
if (ply[player_id]->in_combo && ply[player_id]->acc_jumps > 1)
ply[player_id]->biggest_lost_combo = ply[player_id]->acc_level;

ply[player_id]->in_combo = 0;
ply[player_id]->dead = 1;
play_sound(custom.falling, 0, 1);

endTime = time(0);


for (i = 0; i < 5; i++) {


if (ply[player_id]->jc[i] > ply[player_id]->jcTop[i])
ply[player_id]->jcTop[i] = ply[player_id]->jc[i];


ply[player_id]->jc[i] = 0;
}


jumpSequence.dist = gdLastJumpDiff;
add_jump_sequence(gameData, &jumpSequence);


if (!numComboJumps && ply[player_id]->no_combo_top_floor < ply[player_id]->level)
ply[player_id]->no_combo_top_floor = ply[player_id]->level; lastJumpLength = 0; falling = 1;
}




if (ply[player_id]->y > 900.0 && !game_over) {

play_sound(speaker[1], 0, 0); game_over = 2;
}

if (falling) falling++;
if (falling > ply[player_id]->level * 5 || falling > 250) {
play_sound(sounds[6], 0, 1);
if (custom.falling)
stop_sample(custom.falling);


ply[player_id]->shake = 24; falling = 0;
}



if (ply[player_id]->level >= next_aight) {
play_sound(sounds[2], 0, 0);
if (!options.flash) for (i = 0; i < next_aight / 2; i++) {
int p; p = create_particle(stars, (new_rand() % 600) + 20, 480);
stars[p].sy = -(((new_rand() % 200) << 16) / 10);
}
if (next_aight > 999)
next_aight += 500;
else

next_aight += 50;
}



if (!ply[player_id]->edge) ply[player_id]->edge_drawn = 0;
if (ply[player_id]->edge_drawn) {
if (ply[player_id]->edge_drawn == 11 && !ply[player_id]->status) play_sound(custom.edge, 1, 1);
if (ply[player_id]->edge_drawn == 50)
ply[player_id]->edge_drawn = 0;
}

if (debug) {
if (ply[player_id]->dead <= 99) playing = FALSE;
}




else if (recording && ply[player_id]->dead > 100) playing = FALSE;





if (!itrcheck && key[KEY_F1]) {
int pauseTime = time(NULL);
take_screenshot(swap_screen);
while (key[KEY_F1]) ;
int addTime = time(NULL) - pauseTime;
if (addTime > 0)
startTime += addTime;






if (checkMusicVoiceID >= 0) {

musicCounter = voice_get_position(checkMusicVoiceID) * 50.0f / 44000.0f; totMusics = 0; accMusics = 0.0f;
}




clockTimeStart = clock();

QueryPerformanceCounter(&li);
qpc_start = li.LowPart;



timeTimeStart = time(NULL); time_cheat_count = 0;
}


if (ply[player_id]->shake || ((unsigned char)ply[player_id]->shake && (unsigned short)ply[player_id]->shake && ply[player_id]->shake)) {
ply[player_id]->shake--;

shake = new_rand() % 8;
}

update_frame();



if (!quit && closeButtonClicked) { quit = 1; playing = FALSE;
}



if (recording) {
if (key[KEY_ESC]) {
if (ply[player_id]->dead) {
log2file("  player quit after dying"); playing = 0;
} else {



int pauseTime = time(NULL);
int fc = fall_count;
int ca = clock_angle;
log2file("  game paused with esc");

for (i = 0; i < 640; i += 2) {
vline(swap_screen, i, 0, 480, 0);
hline(swap_screen, 0, i, 640, 0);
}
textout_centre_ex(swap_screen, data[50].dat, "DO YOU REALLY WANT TO EXIT?", 320, 160, -1, -1);
textout_centre_ex(swap_screen, data[52].dat, "Press any key to resume", 320, 210, -1, -1);
textout_centre_ex(swap_screen, data[52].dat, "Press ESC to exit", 320, 240, -1, -1);
blit_to_screen(swap_screen);
play_sound(custom.wazup, 0, 1);

poll_control(&ctrl, 0);
while (is_any(&ctrl) || is_pause(&ctrl) || (!closeButtonClicked && key[KEY_ESC])) {
poll_control(&ctrl, 0);
rest(2);
}
clear_keybuf();
while (!keypressed() && !is_any(&ctrl) && !is_pause(&ctrl) && !closeButtonClicked && !key[KEY_ESC]) {
poll_control(&ctrl, 0);
rest(2);
}
while (!closeButtonClicked && is_pause(&ctrl)) {
poll_control(&ctrl, 0);
rest(2);
}
if (key[KEY_ESC]) {



log2file("  game quit from esc pause");
profile->games_quit++;
endTime = time(NULL); quit = 1; playing = 0;
}
clear_keybuf();

fall_count = fc;
clock_angle = ca;
log2file("  game unpaused");
int addTime = time(NULL) - pauseTime;
if (addTime > 0)
startTime += addTime;






if (checkMusicVoiceID >= 0) {

musicCounter = (int)(voice_get_position(checkMusicVoiceID) * 50.0 / 44000.0); totMusics = 0; accMusics = 0.0f;
}



clockTimeStart = clock();

QueryPerformanceCounter(&li);
qpc_start = li.LowPart;



timeTimeStart = time(NULL); time_cheat_count = 0;
}
}

if (is_pause(&ctrl) && ply[player_id]->dead == 0) {
int pauseTime = time(NULL);
int fc = fall_count;
int ca = clock_angle;
log2file("  game paused with pause key");

for (i = 0; i < 640; i += 2) {
vline(swap_screen, i, 0, 480, 0);
hline(swap_screen, 0, i, 640, 0);
}
textout_centre_ex(swap_screen, data[50].dat, "Game Paused", 320, 160, -1, -1);
textout_centre_ex(swap_screen, data[52].dat, "Press any key to resume", 320, 210, -1, -1);
blit_to_screen(swap_screen);
play_sound(custom.wazup, 0, 1);
poll_control(&ctrl, 0);
while (is_any(&ctrl) || is_pause(&ctrl)) {
poll_control(&ctrl, 0);
rest(2);
}

clear_keybuf();
while (!keypressed()) { if (is_any(&ctrl) || is_pause(&ctrl) || key[KEY_ESC]) break;
poll_control(&ctrl, 0);
rest(2);
}

poll_control(&ctrl, 0);
while (is_pause(&ctrl) || key[KEY_ESC]) {
poll_control(&ctrl, 0);
rest(2);
}


fall_count = fc;
clock_angle = ca;
log2file("  game unpaused");
int addTime = time(NULL) - pauseTime;
if (addTime > 0)
startTime += addTime;






if (checkMusicVoiceID >= 0) {

musicCounter = (int)(voice_get_position(checkMusicVoiceID) * 50.0 / 44000.0); totMusics = 0; accMusics = 0.0f;
}



clockTimeStart = clock();

QueryPerformanceCounter(&li);
qpc_start = li.LowPart;



timeTimeStart = time(NULL); time_cheat_count = 0;
}
}
else {
if (!itrcheck) {

poll_control(&rec_ctrl, 0);

if (ply[player_id]->dead) {

log2file("  replay ended after death"); playing = 0;
}







if (key[KEY_ESC]) {
log2file("  quit from replay"); quit = 1; playing = 0;
}




if (key[KEY_SPACE]) { if (ply[player_id]->dead == 0) {
log2file("  replay paused");
while (key[KEY_SPACE]) poll_control(&rec_ctrl, 1);
while (!key[KEY_SPACE] && !key[KEY_RIGHT] && !key[KEY_ESC] && !key[KEY_UP]) {
poll_control(&rec_ctrl, 1);
if (key[KEY_F1]) {
take_screenshot(swap_screen);
while (key[KEY_F1]) ;
}
}
while (key[KEY_SPACE]) poll_control(&rec_ctrl, 1);
fast_forward = 0;
fast_fast_forward = 0;
log2file("  replay unpaused");
}
}
if (key[KEY_RIGHT]) {
fast_forward++;
fast_fast_forward = 0;
}
else
fast_forward = 0;


if (key[KEY_UP]) {
if (!ply[player_id]->dead && ply[player_id]->level < demo->floor - 10) {
fast_fast_forward++;
fast_forward = 0;
next_floor = ((ply[player_id]->level + 100) / 100) * 100;

next_floor = MIN(next_floor, demo->floor - 10); } else { fast_fast_forward = 0; next_floor = -1;
}
}





else if (ply[player_id]->level >= next_floor || ply[player_id]->dead) {
fast_fast_forward = 0; next_floor = -1;
}
}
}





if (!itrcheck) {
static int someCounter;
int ffstep;
int drew;

someCounter++;


ffstep = fast_forward ? 4 : 1;


if (fast_fast_forward) ffstep = 32;



int skipDrawing;


if (!quit && someCounter % ffstep == 0) {
draw_frame(swap_screen);







if (ply[player_id]->shake) { acquire_screen();

blit(swap_screen, swap_screen, 0, shake, 0, 0, swap_screen->w, swap_screen->h);
blit_to_screen(swap_screen); release_screen();
} else {


blit_to_screen(swap_screen);
}

if (!debug) {
while (cycle_count == 0) rest(2);


} else if (key[KEY_TAB] && key[KEY_LSHIFT]) {
while (cycle_count <= 7) { }
} else {
while (!cycle_count) rest(2);
}
}
}


if (!itrcheck) rest(2);
}
}


if (recording) {
int addTime = endTime - startTime;
if (addTime > 0)
profile->seconds_spent_playing += addTime;
} else {










gameData->score = ply[player_id]->level * 10 + ply[player_id]->score;
gameData->floor = ply[player_id]->level;
gameData->combo = ply[player_id]->best_combo;
gameData->no_combo_top_floor = ply[player_id]->no_combo_top_floor;
gameData->biggest_lost_combo = ply[player_id]->biggest_lost_combo;

for (i = 0; i < 5; i++) gameData->ccc[i] = ply[player_id]->ccc[i];


for (i = 0; i < 5; i++) gameData->jc[i] = ply[player_id]->jcTop[i];
{


int keys_pressed[7] = {0};
int key_flag[7] = { 16, 1, 2, 4, 8, 32, 128 };
int last_keys[7] = {0};

if (demo->size > 0) { for (i = 0; i < demo->size; i++) {
int k;
for (k = 0; k < 7; k++) {
if (!last_keys[k] && (key_flag[k] & demo->data[i].key_flags))
keys_pressed[k]++;

last_keys[k] = key_flag[k] & demo->data[i].key_flags;
}
}
}
gameData->jump = keys_pressed[0];
gameData->left = keys_pressed[1];
gameData->right = keys_pressed[2];


if (itrcheck) {
char *xmlStr = getGameDataXML(gameData);
printf("%s", xmlStr);
free(xmlStr);
}
} if (itrcheck) return 0;
}




























log2file(" play ended");
fast_forward = 0;
fast_fast_forward = 0;









































if (recording) { if (!quit) {


demo->score = ply[player_id]->level * 10 + ply[player_id]->score;
demo->floor = ply[player_id]->level;
demo->combo = ply[player_id]->best_combo;
demo->rejump = options.jump_hold;
demo->no_combo_top_floor = ply[player_id]->no_combo_top_floor;
demo->biggest_lost_combo = ply[player_id]->biggest_lost_combo;

for (i = 0; i < 5; i++) demo->ccc[i] = ply[player_id]->ccc[i];


for (i = 0; i < 5; i++) demo->jc[i] = ply[player_id]->jcTop[i];





if (!is_playing_custom_game) {
profile->games_played++;

profile->total_floors += demo->floor;
profile->total_score += demo->score;
profile->total_combos += numComboJumps;
profile->total_combo_floors += totComboFloors;
for (i = 0; i < 5; i++) {
if (demo->ccc[i] > 0) {
profile->cccNum[i]++;
profile->cccTotal[i] += demo->ccc[i];
}
}
} else {

profile->custom_games_played++;
}





if (!file_exists(replay_directory, -1, NULL))

mkdir(replay_directory);






if (!is_playing_custom_game) {
if (profile->best_floor < demo->floor) {
profile->best_floor = demo->floor;
myDeleteFile(replay_directory, profile->best_replay_names[2]);
sprintf(profile->best_replay_names[2], "%s_best_floor_%d.itr", profile->handle, demo->floor);
save_replay(replay_directory, profile->best_replay_names[2], demo, rec_pos + 2, 1);
new_personal_best[2] = 1;
}

if (profile->best_combo < demo->combo) {
profile->best_combo = demo->combo;
myDeleteFile(replay_directory, profile->best_replay_names[1]);
sprintf(profile->best_replay_names[1], "%s_best_combo_%d.itr", profile->handle, demo->combo);
save_replay(replay_directory, profile->best_replay_names[1], demo, rec_pos + 2, 1);
new_personal_best[1] = 1;
}

if (profile->best_score < demo->score) {
profile->best_score = demo->score;
myDeleteFile(replay_directory, profile->best_replay_names[0]);
sprintf(profile->best_replay_names[0], "%s_best_score_%d.itr", profile->handle, demo->score);
save_replay(replay_directory, profile->best_replay_names[0], demo, rec_pos + 2, 1);
new_personal_best[0] = 1;
}

if (profile->no_combo_top_floor < ply[player_id]->no_combo_top_floor) {
profile->no_combo_top_floor = ply[player_id]->no_combo_top_floor;
myDeleteFile(replay_directory, profile->best_replay_names[4]);
sprintf(profile->best_replay_names[4], "%s_best_no_combo_%d.itr", profile->handle, demo->no_combo_top_floor);
save_replay(replay_directory, profile->best_replay_names[4], demo, rec_pos + 2, 1);
new_personal_best[4] = 1;
}

if (profile->biggest_lost_combo < ply[player_id]->biggest_lost_combo) {
profile->biggest_lost_combo = ply[player_id]->biggest_lost_combo;
myDeleteFile(replay_directory, profile->best_replay_names[3]);
sprintf(profile->best_replay_names[3], "%s_best_lost_combo_%d.itr", profile->handle, demo->biggest_lost_combo);
save_replay(replay_directory, profile->best_replay_names[3], demo, rec_pos + 2, 1);
new_personal_best[3] = 1;
}

for (i = 1; i < 6; i++) {
if (profile->ccc[i - 1] < ply[player_id]->ccc[i - 1]) {
profile->ccc[i - 1] = ply[player_id]->ccc[i - 1];
myDeleteFile(replay_directory, profile->best_replay_names[4 + i]);
sprintf(profile->best_replay_names[4 + i], "%s_best_cc%d_%d.itr", profile->handle, i, ply[player_id]->ccc[i - 1]);
save_replay(replay_directory, profile->best_replay_names[4 + i], demo, rec_pos + 2, 1);
new_personal_best[4 + i] = 1;
}
}

for (i = 1; i < 6; i++) {
if (profile->jc[i - 1] < ply[player_id]->jcTop[i - 1]) {
profile->jc[i - 1] = ply[player_id]->jcTop[i - 1];
myDeleteFile(replay_directory, profile->best_replay_names[9 + i]);
sprintf(profile->best_replay_names[9 + i], "%s_best_jj%d_%d.itr", profile->handle, i, ply[player_id]->jcTop[i - 1]);
save_replay(replay_directory, profile->best_replay_names[9 + i], demo, rec_pos + 2, 1);
new_personal_best[9 + i] = 1;
}
}
}


if (save_replay(replay_directory, "last_game.itr", demo, rec_pos + 2, 1) < 0) {
my_alert("Failed to save replay.", "(last_game.itr)", 0, 1);
uberChecksum = 0;
} else {


char fbuf[2048];
sprintf(fbuf, "%slast_game.itr", replay_directory);
Treplay *rr; rr = load_replay(fbuf);
if (rr) {
uberChecksum = calc_replay_checksum(demo);
destroy_replay(rr);
}
}
}
}











syncProfileFromOptions();
save_profile(profile);
play_again = 0;
if (!quit && !closeButtonClicked) {
float hy;
int gotHigh; gotHigh = 0;


int qualify[15];
int qualifyValue[15];
for (i = 0; i < 15; i++) qualify[i] = 0;

qualifyValue[0] = ply[player_id]->level * 10 + ply[player_id]->score;
qualifyValue[2] = ply[player_id]->level;
qualifyValue[1] = ply[player_id]->best_combo;
qualifyValue[3] = ply[player_id]->biggest_lost_combo;
qualifyValue[4] = ply[player_id]->no_combo_top_floor;
for (i = 0; i < 5; i++) {
qualifyValue[5 + i] = ply[player_id]->ccc[i];
qualifyValue[10 + i] = ply[player_id]->jcTop[i];
}

for (i = 0; i < 15; i++) {
qualify[i] = qualify_hisc_table(hisc_tables[i], qualifyValue[i]);
gotHigh += qualify[i];
}


if (!recording) gotHigh = 0;

int gameover_bmp_id; gameover_bmp_id = gotHigh > 0 ? 62 : 55;
if (is_playing_custom_game) gameover_bmp_id = 0x37;

if (gotHigh && !is_playing_custom_game) {
log2file(" player qualified for highscore");
play_sound(sounds[7], 0, 0);
} else {

log2file(" player did not qualify for highscore");
play_sound(speaker[1], 0, 0);
}


if (!debug) {
int done = 20;
int alpha_pos = 0;
int pos = 0;
char letters[31] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ .\244\0";
int len = strlen(letters) - 1;
char buf[8] = { '.', 0, '.', 0, '.', 0, 0, 0 };
int skip_keys = 0;

int isGuest = !stricmp(profile->handle, "guest");
hy = 480.0f;
float hyTarget = 140.0f;
while (hy > hyTarget) {
cycle_count = 0;
ply[player_id]->dead -= 16;

hy = hy + (130.0f - hy) * 0.1;
update_frame();
for (i = 0; i < 512; i++) if (stars[i].intensity) update_particle(&stars[i]);
if (hurry_y > -100 && hurry_y < 480) hurry_y -= 2;
draw_frame(swap_screen);
draw_results(swap_screen, data[gameover_bmp_id].dat, (int)hy, qualify, qualifyValue, is_playing_custom_game ? 0 : (recording != 0));
if (isGuest && gotHigh && !is_playing_custom_game && recording) {
textout_centre_ex(swap_screen, data[52].dat, "Enter your initials", 320, (int)(hy * 2 + 80), -1, -1);
}

if (falling) falling++;
if (falling > ply[player_id]->level * 5 || falling > 250) {
play_sound(sounds[6], 0, 1);
if (custom.falling) stop_sample(custom.falling);

ply[player_id]->shake = 24; falling = 0;
}
if (ply[player_id]->shake) { acquire_screen();

blit(swap_screen, screen, 0, new_rand() % 8, 0, 0, swap_screen->w, swap_screen->h); release_screen();

ply[player_id]->shake--; } else

blit_to_screen(swap_screen);


if (key[KEY_F1]) {
take_screenshot(swap_screen);
while (key[KEY_F1]) { }
}


if (!key[KEY_TAB] || !key[KEY_LSHIFT])


while (!cycle_count) rest(2);
}

ply[player_id]->dead = 0;
clear_keybuf();



if (!recording)
summary_scroller_message[0] = 0;
else {
if (is_playing_custom_game) {
strcpy(summary_scroller_message, "Custom mode is crazy fun but does not add to your profile. " "Play Classic Mode to compete in the highscore lists and " "climb in rank!");
} else {

if (gotHigh) {
int skipCategories[5];
int h;
int achs;
strcpy(summary_scroller_message, "New personal records!    ");
















strcpy(summary_scroller_message, isGuest ? "You're playing in guest mode. Start a profile and record your progress!" : hints[new_rand() % 45]);



} else {
strcpy(summary_scroller_message, isGuest ? "You're playing in guest mode. Start a profile and record your progress!" : hints[new_rand() % 45]);
}
}
}


init_scroller(&summary_scroller, data[54].dat, summary_scroller_message, 640, 30, -1);
scroll_scroller(&summary_scroller, -150);
int scrollerTargetY = 0;
int scrollerY = -20;


int new_rank_id; new_rank_id = get_rank_id(profile);
int rank_bmp_id; rank_bmp_id = new_rank_id + 0x4a;
int rank_y; rank_y = 0x244;
int rankTargetY = 320;

while (done) {
if (closeButtonClicked) return 0;



cycle_count = 0;
step_count++;

update_frame();

if (key[KEY_F1]) {
take_screenshot(swap_screen);
while (key[KEY_F1]) { }
}


if (hurry_y > -100 && hurry_y < 480) hurry_y -= 2;
draw_frame(swap_screen);
draw_results(swap_screen, data[gameover_bmp_id].dat, (int)hy, qualify, qualifyValue, is_playing_custom_game ? 0 : (recording != 0));
if (isGuest && gotHigh && !is_playing_custom_game && recording) {
textout_centre_ex(swap_screen, data[52].dat, "Enter your initials", 320, (int)(hy * 2 + 80), -1, -1);

if (pos != 0 || (step_count & 4)) textout_centre_ex(swap_screen, data[52].dat, &buf[0], 300, (int)(hy * 2 + 120), -1, -1);
if (pos != 1 || (step_count & 4)) textout_centre_ex(swap_screen, data[52].dat, &buf[2], 320, (int)(hy * 2 + 120), -1, -1);
if (pos != 2 || (step_count & 4)) textout_centre_ex(swap_screen, data[52].dat, &buf[4], 340, (int)(hy * 2 + 120), -1, -1);
if (pos == 3 && (step_count & 4)) textout_centre_ex(swap_screen, data[52].dat, "%", 360, (int)(hy * 2 + 120), -1, -1);
}

if (new_rank_id != current_rank_id) {
draw_sprite(swap_screen, data[rank_bmp_id].dat, 20, rank_y);
textout_ex(swap_screen, data[52].dat, "rank up!", 20, rank_y + 0x46, -1, -1);
rank_y = (int)((rankTargetY - rank_y) * 0.1 + rank_y);
}


if (summary_scroller_message[0]) {
scroll_scroller(&summary_scroller, -2);
drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
set_trans_blender(0, 0, 0, 110);
rectfill(swap_screen, 0, scrollerY, 639, scrollerY + 20, makecol(0, 0, 0));
rectfill(swap_screen, 0, scrollerY, 639, scrollerY + 18, makecol(0, 0, 0));
rectfill(swap_screen, 0, scrollerY, 639, scrollerY + 16, makecol(0, 0, 0));
solid_mode();
draw_scroller(&summary_scroller, swap_screen, 1, scrollerY, makecol(150, 150, 150));
if (!draw_scroller(&summary_scroller, swap_screen, 0, scrollerY, makecol(200, 200, 200))) restart_scroller(&summary_scroller);

scrollerY = (int)((scrollerTargetY - scrollerY) * 0.1 + scrollerY);
}



if (falling) falling++;
if (falling > ply[player_id]->level * 5 || falling > 250) {
play_sound(sounds[6], 0, 1);
if (custom.falling)
stop_sample(custom.falling);


ply[player_id]->shake = 24; falling = 0;
}
if (ply[player_id]->shake) { acquire_screen();


blit(swap_screen, screen, 0, new_rand() % 8, 0, 0, swap_screen->w, swap_screen->h); release_screen();

ply[player_id]->shake--; } else


blit_to_screen(swap_screen);


if (isGuest && gotHigh && !is_playing_custom_game && recording) {
poll_control(&ctrl, 0);
if (keypressed()) { if (done == 20) {
int k; k = readkey() & 0xff; k -= 0x20;
if (k == -24) k = (signed char)0xa4;
if (k == 14) k = '.';
if (k == 1) k = '!';
if (k != 0x20) {
for (i = 0; i < len; i++) {
if (letters[i] == k) {


buf[pos * 2] = letters[i];
pos++; alpha_pos = i; skip_keys = 100;
if (pos == 3) done = 19;
}
}
}
}
}
if (skip_keys) { skip_keys--; } else {
if (is_right(&ctrl)) {
alpha_pos++; skip_keys = 8;
if (alpha_pos > len) alpha_pos = 0;
}

if (is_left(&ctrl)) { skip_keys = 8;

alpha_pos--; if (alpha_pos < 0) alpha_pos = len;
}

if (is_fire(&ctrl)) {
if (letters[alpha_pos] == (char)0xa4) { if (pos != 0) {
buf[pos * 2] = '.';
pos--; } skip_keys = 100;

} else if (pos <= 1) { pos++; skip_keys = 100; } else {
if (done == 20) {

pos++; done = 19; } skip_keys = 100;
}
}

if (key[KEY_DEL] || key[KEY_BACKSPACE]) {
buf[pos * 2] = '.'; skip_keys = 7;
if (pos != 0) pos--; } else if (skip_keys) {



skip_keys--; } }
if (!is_any(&ctrl) && !key[KEY_DEL] && !key[KEY_BACKSPACE]) skip_keys = 0;

if (pos <= 2) buf[pos * 2] = letters[alpha_pos];
}

if (done != 20) done--;

poll_control(&ctrl, 0);
if (!isGuest || !gotHigh || is_playing_custom_game) {
if (keypressed() || is_fire(&ctrl)) {
if (done == 20) done = 14;
}
}



if (!key[KEY_TAB] || !key[KEY_LSHIFT])


while (!cycle_count) rest(2);

}





if (!is_playing_custom_game && recording) {
char guestName[4]; guestName[0] = buf[0]; guestName[1] = buf[2]; guestName[2] = buf[4]; guestName[3] = 0;
char postName[32];
strcpy(postName, isGuest ? guestName : profile->handle);




for (i = 0; i < 15; i++) {
if (qualify[i] > 0) {
enter_hisc_table(hisc_tables[i], qualifyValue[i], postName);
sort_hisc_table(hisc_tables[i]);
}
}
}

}






if (recording && !debug && !is_playing_custom_game) {
int f = ply[player_id]->level / 100;

if (f > oldUnlockedFloors && f <= 9) {

fadeOut(16);


blit(data[126].dat, swap_screen, 0, 0, 0, 0, 640, 480);


set_trans_blender(0, 0, 0, 158);
drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
rectfill(swap_screen, 0, 0, SCREEN_W, SCREEN_H, makecol(0, 0, 0));
solid_mode();


draw_sprite(swap_screen, data[58].dat, 320 - ((BITMAP *)data[58].dat)->w / 2, 20);
textout_centre_ex(swap_screen, data[52].dat, "A new start floor", 320, 0x12c, -1, -1);
textout_centre_ex(swap_screen, data[52].dat, "has been unlocked!", 320, 0x15e, -1, -1);
textout_centre_ex(swap_screen, data[52].dat, "(Get it in the options menu)", 320, 0x1b8, -1, -1);
play_sound(sounds[2], 0, 0);
fadeIn(swap_screen, 16);
while (key[KEY_ESC] || (key[KEY_ENTER] | key[KEY_SPACE])) { }
while (!key[KEY_ESC] && !key[KEY_ENTER] && !key[KEY_SPACE]) { }
}
}




save_config();


stopGameMusic();
if (checkMusicVoiceID >= 0)
voice_stop(checkMusicVoiceID);


if (recording) { if (!debug) {
in_replay_menu = 1;
play_again = do_replay_menu();
in_replay_menu = 0;
}
}
}


if (recording) play_sound(speaker[2], 0, 0);

stopGameMusic();
if (checkMusicVoiceID >= 0)
voice_stop(checkMusicVoiceID);


clear(screen);

return play_again;
}