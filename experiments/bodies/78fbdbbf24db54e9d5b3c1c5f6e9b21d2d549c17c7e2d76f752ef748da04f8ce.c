{
    char *data;
    int seconds;
    int minutes;
    int hours;
    int days;
    char timeSpent[256];
    char buf[80];
    int len;
    int i;

    data = malloc(2048);
    data[0] = 0;
    sprintf(data, "%sCreated:        %s%s\n", data, filler, p->creationDate);
    sprintf(data, "%s\n", data);
    seconds = p->seconds_spent_playing % 60;
    minutes = (p->seconds_spent_playing / 60) % 60;
    hours = (p->seconds_spent_playing / 3600) % 24;
    days = p->seconds_spent_playing / 86400;
    strcpy(timeSpent, "none");
    if (seconds > 0)
        sprintf(timeSpent, "%d second%s", seconds, seconds > 1 ? "s" : "");
    if (minutes > 0)
        sprintf(timeSpent, "%d minute%s, %d second%s", minutes,
                minutes > 1 ? "s" : "", seconds,
                seconds > 1 ? "s" : "");
    if (hours > 0)
        sprintf(timeSpent, "%d hour%s, %d minute%s, %d second%s", hours,
                hours > 1 ? "s" : "", minutes,
                minutes > 1 ? "s" : "", seconds,
                seconds > 1 ? "s" : "");
    if (days > 0)
        sprintf(timeSpent, "%d day%s, %d hour%s, %d minute%s, %d second%s",
                days, days > 1 ? "s" : "", hours,
                hours > 1 ? "s" : "", minutes,
                minutes > 1 ? "s" : "", seconds,
                seconds > 1 ? "s" : "");
    sprintf(data, "%sTime played:    %s%s\n", data, filler, timeSpent);
    sprintf(buf, "%d", p->games_played);
    len = strlen(buf);
    buf[0] = 0;
    for (i = 10; i >= len; i--)
        strcat(buf, " ");
    sprintf(data, "%sGames played:   %s%d     %s  Custom games played:  %5d\n",
            data, filler, p->games_played, buf, p->custom_games_played);
    sprintf(data, "%sGames quit:     %s%d\n", data, filler, p->games_quit);
    sprintf(data, "%s\n", data);
    return data;
}