

// Convenience functions
unsigned char clamp_u8(unsigned char value, unsigned char min, unsigned char max);
unsigned short clamp_u16(unsigned short value, unsigned short min, unsigned short max);


struct razer_report {
    unsigned char status;
    union transaction_id_union transaction_id; /* */
    unsigned short remaining_packets; /* Big Endian */
    unsigned char protocol_type; /*0x0*/
    unsigned char data_size;
    unsigned char command_class;
    union command_id_union command_id;
    unsigned char arguments[80];
    unsigned char crc;/*xor'ed bytes of report*/
    unsigned char reserved; /*0x0*/
};



/**
 * Get initialised razer report
 */
struct razer_report get_razer_report(unsigned char command_class, unsigned char command_id, unsigned char data_size)
{
    struct razer_report new_report = {0};
    memset(&new_report, 0, sizeof(struct razer_report));

    new_report.status = 0x00;
    new_report.transaction_id.id = 0xFF;
    new_report.remaining_packets = 0x00;
    new_report.protocol_type = 0x00;
    new_report.command_class = command_class;
    new_report.command_id.id = command_id;
    new_report.data_size = data_size;

    return new_report;
}


/**
 * Sets up the extended matrix effect payload
 */
struct razer_report razer_chroma_extended_matrix_effect_base(unsigned char arg_size, unsigned char variable_storage, unsigned char led_id, unsigned char effect_id)
{
    struct razer_report report = get_razer_report(0x0F, 0x02, arg_size);
    report.transaction_id.id = 0x3F;

    report.arguments[0] = variable_storage;
    report.arguments[1] = led_id;
    report.arguments[2] = effect_id; // Effect ID

    return report;
}




/**
 * Set the effect of the LED matrix to Starlight
 *
 * Status Trans Packet Proto DataSize Class CMD Args
 * ? TODO fill this
 */
struct razer_report razer_chroma_standard_matrix_effect_starlight_single(unsigned char variable_storage, unsigned char led_id, unsigned char speed, struct razer_rgb *rgb1)
{
    struct razer_report report = get_razer_report(0x03, 0x0A, 0x01);

    speed = clamp_u8(speed, 0x01, 0x03); // For now only seen

    report.arguments[0] = 0x19; // Effect ID
    report.arguments[1] = 0x01; // Type one color
    report.arguments[2] = speed; // Speed

    report.arguments[3] = rgb1->r; // Red 1
    report.arguments[4] = rgb1->g; // Green 1
    report.arguments[5] = rgb1->b; // Blue 1

    // For now haven't seen any chroma using this, seen the extended version
    report.arguments[6] = 0x00; // Red 2
    report.arguments[7] = 0x00; // Green 2
    report.arguments[8] = 0x00; // Blue 2

    return report;
}

/**
 * Set the effect of the LED matrix to Starlight
 *
 * Status Trans Packet Proto DataSize Class CMD Args
 * ? TODO fill this
 */
struct razer_report razer_chroma_standard_matrix_effect_starlight_dual(unsigned char variable_storage, unsigned char led_id, unsigned char speed, struct razer_rgb *rgb1, struct razer_rgb *rgb2)
{
    struct razer_report report = get_razer_report(0x03, 0x0A, 0x01);

    speed = clamp_u8(speed, 0x01, 0x03); // For now only seen

    report.arguments[0] = 0x19; // Effect ID
    report.arguments[1] = 0x02; // Type two color
    report.arguments[2] = speed; // Speed

    report.arguments[3] = rgb1->r; // Red 1
    report.arguments[4] = rgb1->g; // Green 1
    report.arguments[5] = rgb1->b; // Blue 1

    report.arguments[6] = rgb2->r; // Red 2
    report.arguments[7] = rgb2->g; // Green 2
    report.arguments[8] = rgb2->b; // Blue 2

    return report;
}

//my modifications
struct razer_report first_effect(unsigned char variable_storage, unsigned char led_id, unsigned char speed, struct razer_rgb *rgb1, struct razer_rgb *rgb2)
{
    struct razer_report report = get_razer_report(0x03, 0x0A, 0x01);

    speed = clamp_u8(speed, 0x01, 0x03); // For now only seen

    report.arguments[0] = 0x19; // Effect ID
    report.arguments[1] = 0x02; // Type two color
    //report.arguments[1] = 0x03; //will this allow 3 colors?
    report.arguments[2] = speed; // Speed

    
    //dual colors: red and blue
    report.arguments[3] = rgb1->r; // Red 1
    report.arguments[4] = rgb1->r; 
    report.arguments[5] = rgb1->r; 

    report.arguments[6] = rgb2->b; 
    report.arguments[7] = rgb2->b; 
    report.arguments[8] = rgb2->b; // Blue 2
    
    //remove comments below for 3 color capability
    /*
    report.arguments[9] = rgb2->g;
    report.arguments[10] = rgb2->g;
    report.arguments[11] = rgb2->g; //green
    */

    return report;
}



//info/instructions for functions below can be found @ line 418 on razerchromacommon.c

//play around with setting different colors to different colors
//can maybe have two functions called by one, like 2 columns 1 colors and 2 colums another color
struct razer_report razer_chroma_extended_matrix_set_custom_frame(unsigned char row_index, unsigned char start_col, unsigned char stop_col, unsigned char *rgb_data)
{
    return razer_chroma_extended_matrix_set_custom_frame2(row_index, start_col, stop_col, rgb_data, 0x47);
}

struct razer_report razer_chroma_extended_matrix_set_custom_frame2(unsigned char row_index, unsigned char start_col, unsigned char stop_col, unsigned char *rgb_data, size_t packetLength)
{
    const size_t row_length = (size_t) (((stop_col + 1) - start_col) * 3);
    // Some devices need a specific packet length, most devices are happy with 0x47
    // e.g. the Mamba Elite needs a "row_length + 5" packet length
    const size_t data_length = (packetLength != 0) ? packetLength : row_length + 5;
    struct razer_report report = get_razer_report(0x0F, 0x03, data_length);

    report.transaction_id.id = 0x3F;

    // printk(KERN_ALERT "razerkbd: Row ID: %d, Start: %d, Stop: %d, row length: %d\n", row_index, start_col, stop_col, (unsigned char)row_length);

    report.arguments[2] = row_index;
    report.arguments[3] = start_col;
    report.arguments[4] = stop_col;
    memcpy(&report.arguments[5], rgb_data, row_length);

    return report;
}