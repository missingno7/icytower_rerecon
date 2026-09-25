{
    int i;
    unsigned int sum;

    sum = r->biggest_lost_combo * 17 + 17;
    sum += r->no_combo_top_floor * 127;
    sum += r->floor_shrink * 102 + r->floor_size * 17 + 3702;
    sum += r->start_speed * 163 + r->speed_increase * 23;
    sum += r->gravity * 88 + r->random_seed * 329;
    sum += r->tc_posts * 127 + r->rejump * 13;
    sum += r->score * 17 + 17;
    sum += (r->combo + 1) * 649;
    sum += (r->floor + 1) * 113;
    for (i = 0; i < 5; i++)
        sum += r->ccc[i] * (39 + i * 3) + r->jc[i] * (27 + i * 3);
    for (i = 0; i < 100; i++) {
        sum += r->tc_c_data[i] * ((i + 1) % 13);
        sum += r->tc_q_data[i] * ((i + 6) % 17);
        sum += r->tc_t_data[i] * ((i + 8) % 23);
    }
    for (i = 0; i < 32; i++)
        sum += (r->date[i] + i) * (r->name[i] + i) * (17 + i * 17);
    for (i = 0; i < 42; i++)
        sum += (r->comment[i] + i) * (r->comment[i] + i) * (-3 + i * 3);
    for (i = 0; i < r->size; i++)
        sum += r->data[i].cycle_count * 7 * (i % 167 + 1) +
               r->data[i].key_flags * 3 * (i % 193 + 1);
    return hash(sum);
}