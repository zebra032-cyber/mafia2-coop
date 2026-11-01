// =======================================================================//
// !
// ! General stuff
// !
// =======================================================================//

void on_car_create_command(librg_message_t *msg) {
    auto player     = librg_entity_find(msg->ctx, msg->peer);

    /* model id sent by the client (debug menu / F10); older clients send nothing -> 10 */
    u16 model = 10;
    if (librg_data_get_rpos(msg->data) + sizeof(u16) <= librg_data_capacity(msg->data)) {
        model = librg_data_ru16(msg->data);
    }
    if (model > 54) model = 10; /* m2o_vehicle_create clamps to 0..54 anyway */

    auto vehicleid  = m2o_vehicle_create(model);

    m2o_vehicle_position_set(vehicleid, vec3f(
        player->position.x + 3.0f,
        player->position.y,
        player->position.z + 0.05f
    ));

    // log
    print_posm(m2o_vehicle_position_get(vehicleid), "created a vehicle at: ");
}

void on_car_create(librg_event_t *event) {
    auto car = m2o_car_get(event->entity);

    librg_data_wu16(event->data, car->model);
    librg_data_wu8(event->data,  car->state);
    librg_data_wptr(event->data, &car->stream, sizeof(car->stream));
}

void on_car_remove(librg_event_t *event) {

}

// =======================================================================//
// !
// ! Vehicle events
// !
// =======================================================================//

void on_car_enter(librg_message_t *msg) {
    auto player  = librg_entity_find(msg->ctx, msg->peer);
    auto vehicle = librg_entity_fetch(msg->ctx, librg_data_ru32(msg->data));
    mod_assert_msg(vehicle && player, "trying to enter invalid vehicle");

    auto seat = librg_data_ru8(msg->data);
    auto ped  = m2o_ped_get(player);

    /* only one driver per car: if someone else already drives it, this player rides along */
    if (seat == 1) {
        bool taken = false;
        librg_entity_iteratex(msg->ctx, LIBRG_ENTITY_ALIVE, otherid, {
            auto other = librg_entity_fetch(msg->ctx, otherid);
            if (!other || other == player || other->type != M2O_ENTITY_PLAYER_PED) continue;
            auto oped = m2o_ped_get(other);
            if (oped && oped->state == PED_IN_CAR && oped->vehicle == vehicle->id && oped->seat == 1) taken = true;
        });
        if (taken) {
            mod_log("[warning] ped %u tried to take driver seat of %u which is occupied - treating as passenger\n", player->id, vehicle->id);
            seat = 2;
        }
    }

    ped->state   = PED_IN_CAR;
    ped->vehicle = vehicle->id;
    ped->seat    = seat;

    /* if player is in the driver seat - he becomes the vehicle streamer (vacant cars: anyone) */
    if (seat == 1) {
        librg_entity_control_set(msg->ctx, vehicle->id, player->client_peer);
    }

    mod_log("[info] ped: %u becomes member of: %u on seat: %u\n", player->id, vehicle->id, seat);
    mod_message_send_instream_except(msg->ctx, M2O_CAR_ENTER, player->id, player->client_peer, [&](librg_data_t *data) {
        librg_data_wu32(data, player->id);
        librg_data_wu32(data, vehicle->id);
        librg_data_wu8(data, seat);
    });

    m2o_args args = {0};
    m2o_args_init(&args);
    m2o_args_push_integer(&args, player->id);
    m2o_args_push_integer(&args, vehicle->id);
    m2o_args_push_integer(&args, seat);
    m2o_event_trigger(M2O_EVENT_PED_CAR_ENTER, &args);
    m2o_args_free(&args);
}

void on_car_exit(librg_message_t *msg) {
    auto player  = librg_entity_find(msg->ctx, msg->peer);
    auto ped     = m2o_ped_get(player);

    mod_log("player: %d is trying to leave his current car\n", player->id);
    auto old_vehicle = ped->vehicle;
    auto old_seat    = ped->seat;

    /* Control is NOT removed when the driver gets out: the last driver keeps simulating the parked
     * car (no handover mid get-out animation, which crashed both games). It moves to whoever
     * drives it next (on_car_enter), or is dropped when that player disconnects / streams it out. */
    (void)old_seat;

    mod_message_send_instream_except(msg->ctx, M2O_CAR_EXIT, player->id, player->client_peer, [&](librg_data_t *data) {
        librg_data_wu32(data, player->id);
    });

    ped->state   = PED_ON_GROUND;
    ped->vehicle = M2O_INVALID_ENTITY;
    ped->seat    = 0;

    m2o_args args = {0};
    m2o_args_init(&args);
    m2o_args_push_integer(&args, player->id);
    m2o_args_push_integer(&args, old_vehicle);
    m2o_args_push_integer(&args, old_seat);
    m2o_event_trigger(M2O_EVENT_PED_CAR_EXIT, &args);
    m2o_args_free(&args);
}
