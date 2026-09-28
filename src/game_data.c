#include "recovered/Tgame_data.h"
/* Historical CU: F:\projects\icytower\trunk\source\game_data.c
 * Ownership: GAME
 * Partial source recovery.
 * Partial reconstruction of getGameDataXML @ 0x00404254.
 */

#include "recovered/Tgd_combo.h"
#include "recovered/Tgd_jump_sequence.h"
#include "recovered/Treplay.h"

#include "recovered/Tgame_data.h"

#include "recovered/Tcommandline.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern Tcommandline cmdline;

Tgame_data *create_game_data(void)
{
    int i;
    Tgame_data *gd;

    gd=malloc(sizeof(Tgame_data));
    if (gd) {
        gd->replay=0;
        gd->score=gd->floor=gd->combo=gd->no_combo_top_floor=gd->biggest_lost_combo=0;
        for (i=0;i<5;i++) gd->ccc[i]=gd->jc[i]=0;
        gd->comboPosts=0;
        gd->jumpPosts=0;
        gd->left=gd->right=gd->jump=0;
    }
    return gd;
}

char *getGameDataXML(Tgame_data *gd)
{
    char playerTag[256], gameTag[512], claimTag[1024], actualTag[1024];
    /* port: sized for the 5000-entry tables (the historical 5120-byte
     * buffers overflowed on long games) */
    static char comboTag[5000*72+64], jumpTag[5000*72+64], sdTag[100*96+64];
    char keysTag[256];
    char *xmlStr;
    int i;

    xmlStr=malloc(sizeof(comboTag)+sizeof(jumpTag)+sizeof(sdTag)+16384);
    sprintf(playerTag,"  <player>\n    <name>%s</name>\n  </player>\n",gd->replay->name);
    sprintf(gameTag,"  <game>\n    <comment>%s</comment>\n    <settings>\n      <floor_shrink>%d</floor_shrink>\n      <floor_size>%d</floor_size>\n      <speed_increase>%d</speed_increase>\n      <start_speed>%d</start_speed>\n      <gravity>%d</gravity>\n      <rejump>%d</rejump>\n    </settings>\n  </game>\n",gd->replay->comment,gd->replay->floor_shrink,gd->replay->floor_size,gd->replay->speed_increase,gd->replay->start_speed,gd->replay->gravity,gd->replay->rejump);
    sprintf(claimTag,"      <score>%d</score>\n      <floor>%d</floor>\n      <combo>%d</combo>\n      <no_combo_floor>%d</no_combo_floor>\n      <lost_combo>%d</lost_combo>\n",gd->replay->score,gd->replay->floor,gd->replay->combo,gd->replay->no_combo_top_floor,gd->replay->biggest_lost_combo);
    for (i=1;i<6;i++) if (gd->replay->ccc[i-1]>0) sprintf(claimTag+strlen(claimTag),"      <ccc level=\"%d\">%d</ccc>\n",i,gd->replay->ccc[i-1]);
    for (i=1;i<6;i++) if (gd->replay->jc[i-1]>0) sprintf(claimTag+strlen(claimTag),"      <js level=\"%d\">%d</js>\n",i,gd->replay->jc[i-1]);
    sprintf(actualTag,"      <score>%d</score>\n      <floor>%d</floor>\n      <combo>%d</combo>\n      <no_combo_floor>%d</no_combo_floor>\n      <lost_combo>%d</lost_combo>\n",gd->score,gd->floor,gd->combo,gd->no_combo_top_floor,gd->biggest_lost_combo);
    for (i=1;i<6;i++) if (gd->replay->ccc[i-1]>0) sprintf(actualTag+strlen(actualTag),"      <ccc level=\"%d\">%d</ccc>\n",i,gd->ccc[i-1]);
    for (i=1;i<6;i++) if (gd->replay->jc[i-1]>0) sprintf(actualTag+strlen(actualTag),"      <js level=\"%d\">%d</js>\n",i,gd->jc[i-1]);
    sprintf(comboTag,"    <combos>\n");
    for (i=0;i<gd->comboPosts;i++) sprintf(comboTag+strlen(comboTag),"      <combo start=\"%d\" end=\"%d\">%d</combo>\n",gd->combos[i].start,gd->combos[i].end,gd->combos[i].length);
    strcat(comboTag,"    </combos>\n");
    sprintf(jumpTag,"    <jumps>\n");
    for (i=0;i<gd->jumpPosts;i++) sprintf(jumpTag+strlen(jumpTag),"      <sequence dist=\"%d\" start=\"%d\">%d</sequence>\n",gd->jumps[i].dist,gd->jumps[i].start,gd->jumps[i].num);
    strcat(jumpTag,"    </jumps>\n");
    sprintf(keysTag,"  <keys>\n    <left>%d</left>\n    <right>%d</right>\n    <jump>%d</jump>\n  </keys>\n",gd->left,gd->right,gd->jump);
    sprintf(sdTag,"  <sd>\n");
    for (i=0;i<gd->replay->tc_posts;i++) sprintf(sdTag+strlen(sdTag),"    <entry clk=\"%2.2f\" qpc=\"%2.2f\" tme=\"%2.2f\" dns=\"%2.2f\" flr=\"%d\" />\n",gd->replay->tc_c_data[i],gd->replay->tc_q_data[i],gd->replay->tc_t_data[i],gd->replay->tc_s_data[i],(int)gd->replay->tc_f_data[i]);
    strcat(sdTag,"  </sd>\n");
    sprintf(xmlStr,"<itrcheck_results file_status=\"ok\" header=\"%c%c%c%c%c%c\" date=\"%s\">\n",gd->replay->header[0],gd->replay->header[1],gd->replay->header[2],gd->replay->header[3],gd->replay->header[4],gd->replay->header[5],gd->replay->date);
    if (!cmdline.tiny) {
        strcat(xmlStr,playerTag);
        strcat(xmlStr,gameTag);
        strcat(xmlStr,"  <results>\n");
        strcat(xmlStr,"    <claimed_results>\n");
        strcat(xmlStr,claimTag);
        strcat(xmlStr,"    </claimed_results>\n");
        strcat(xmlStr,"    <actual_results>\n");
        strcat(xmlStr,actualTag);
        strcat(xmlStr,"    </actual_results>\n");
        if (cmdline.combos) strcat(xmlStr,comboTag);
        if (cmdline.jumps) strcat(xmlStr,jumpTag);
        strcat(xmlStr,"  </results>\n");
        if (cmdline.keys) strcat(xmlStr,keysTag);
        if (cmdline.sd) strcat(xmlStr,sdTag);
    } else {
        int misses;

        misses=0;
        if (gd->score!=gd->replay->score) misses++;
        if (gd->floor!=gd->replay->floor) misses++;
        if (gd->combo!=gd->replay->combo) misses++;
        if (gd->no_combo_top_floor!=gd->replay->no_combo_top_floor) misses++;
        if (gd->biggest_lost_combo!=gd->replay->biggest_lost_combo) misses++;
        for (i=0;i<5;i++) {
            if (gd->ccc[i]!=gd->replay->ccc[i]) misses++;
            if (gd->jc[i]!=gd->replay->jc[i]) misses++;
        }
        sprintf(xmlStr+strlen(xmlStr),"  <result>%s</result>\n",misses ? "mismatch" : "match");
    }
    strcat(xmlStr,"</itrcheck_results>\n");
    return xmlStr;
}

void add_jump_sequence(Tgame_data *gd,Tgd_jump_sequence *js)
{
    if (js->num && gd->jumpPosts<5000) {
        gd->jumps[gd->jumpPosts].num=js->num;
        gd->jumps[gd->jumpPosts].dist=js->dist;
        gd->jumps[gd->jumpPosts].start=js->start;
        gd->jumpPosts++;
    }
}

void add_combo(Tgame_data *gd,Tgd_combo *c)
{
    if (gd->comboPosts<5000) {
        gd->combos[gd->comboPosts].end=c->end;
        gd->combos[gd->comboPosts].start=c->start;
        gd->combos[gd->comboPosts].length=c->length;
        gd->comboPosts++;
    }
}

void destroy_game_data(Tgame_data *gd) { free(gd); }
