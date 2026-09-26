{
    int num_posts;
    int old_pos;
    int pos;
    int return_value;

    pos = 0;
    num_posts = -1;
    do {
        num_posts++;
        if (m[num_posts].flags & 1)
            pos = num_posts;
    } while ((signed char)m[num_posts].flags >= 0);
    old_pos = pos;
    draw_menu(bmp, m, mp, x, y, stepIn);
    if (ctrl) {
        if (is_up(ctrl) || is_up(&mp->ctrl)) {
            pos--;
            if (pos < 0)
                pos = num_posts;
        }
        if (is_down(ctrl) || is_down(&mp->ctrl)) {
            pos++;
            if (pos > num_posts)
                pos = 0;
        }
        if (key[KEY_F1]) {
            if (pos != num_posts) {
                pos = num_posts;
                return_value = 0;
            } else {
                return_value = m[pos].return_select;
            }
        } else {
            return_value = 0;
        }
    } else {
        return_value = 0;
    }
    if (old_pos != pos) {
        m[old_pos].flags &= ~1;
        m[pos].flags |= 1;
        play_menu_move();
    }
    if (ctrl) {
        if (is_fire(ctrl) || is_enter(&mp->ctrl) ||
            is_fire(&mp->ctrl))
            return_value = m[pos].return_select;
        if (is_left(ctrl) || is_left(&mp->ctrl))
            return_value = m[pos].return_left;
        if (is_right(ctrl) || is_right(&mp->ctrl))
            return_value = m[pos].return_right;
    }
    *data = (int)m[pos].data;
    mp->pos = pos;
    return return_value;
}