{
    int i;
    int width;

    for (i=0; i<31; i++)
        m->room[i]=m->room[i+1];

    m->room[31].tiles = m->room[31].level>4999 ? 10 : m->room[31].level/500;
    if ((m->room[31].level%250==0 && m->room[31].level<=5004) || m->room[31].level%2500==0) {
        m->room[31].empty=0;
        m->room[31].level++;
        m->room[31].start_tile=0;
        m->room[31].end_tile=40;
    }
    else if (m->room[31].level%5==0) {
        m->room[31].empty=0;
        m->room[31].level++;
        if (get_demo()->floor_shrink) {
            if (m->room[31].level < 3000) {
                width=rand();
                if (((float)(300-m->room[31].level/5)/300.0f)*10.0f<1.0f) width=6;
                else {
                    int max_w=(int)(((float)(300-m->room[31].level/5)/300.0f)*10.0f);
                    width=width%max_w+6;
                }
            }
            else if (m->room[31].level<=5004)
                width=6;
            else if (m->room[31].level<=7504)
                width=5;
            else if (m->room[31].level<=10004)
                width=4;
            else
                width=m->room[31].level<50005 ? 3 : 2;
        }
        else width=rand()%10+6;

        width+=floor_size_modifiers[get_demo()->floor_size];
        if (width>0) {
            m->room[31].start_tile=rand()%(30-width)+5;
            m->room[31].end_tile=m->room[31].start_tile+width;
        }
        else {
            m->room[31].start_tile=rand()%29+5;
            m->room[31].end_tile=m->room[31].start_tile+1;
        }
    }
    else {
        m->room[31].empty=-1;
        m->room[31].level++;
    }

    if ((m->room[31].level-1)%50==0)
        m->room[31].sign=m->room[31].level/5;
    else
        m->room[31].sign=0;
}