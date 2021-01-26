% Copy this file to ~/.config/mc/plugin/, it'll be automatically loaded at startup.

define grow_shrink_int__get_current_or_next_number() {
    variable pos = -1, bol_offset, eol_offset, start_digit_offset,
        t = "", found, start_digit_seen, c1,  c2;

    bol_offset = mc->cure_get_bol ();
    eol_offset = mc->cure_get_eol ();

    % Find the start of the number (a word, actually)
    for (pos = mc->cure_cursor_offset(); pos >= bol_offset; pos--)
    {
        c1 = mc->cure_get_byte (pos);
        c2 = mc->cure_get_byte (pos - 1);

        if (not isdigit(c1))
            break;
        if (not isspace (c1) && isspace (c2))
            break;
    }

    % Find the end of the number (a word, actually)
    found=0;
    start_digit_offset=0;
    start_digit_seen=0;
    for (; pos <= eol_offset; pos++)
    {
        c1 = mc->cure_get_byte (pos);
        c2 = mc->cure_get_byte (pos + 1);

        % Append the byte to the string
        if (isdigit (c1)) {
            if (not start_digit_seen) {
                start_digit_seen=1;
                start_digit_offset=pos;
            }
            found = 1; t += char(c1);
        }

        if (isdigit (c1) && not isdigit (c2)) {
            found += 1;
            break;
        }
    }

    % Any number found?
    % If not, return an empty string and minus one position
    if (found == 2) {
        % Include any minus sign
        c1 = mc->cure_get_byte (start_digit_offset-1);
        if (c1 == '-')
            t = char(c1) + t;
    }
    return t, pos;
}

% A backend function for the two next public functions
define grow_shrink_int__increment(direction) {
    variable pos, cpos, number = 0;
    variable number_str, new_number_buf;
    variable number_len = 0, new_number_len = 0, idx;

    % Get the number
    (number_str, pos) = grow_shrink_int__get_current_or_next_number();
    % Has been a number found?
    if (strlen(number_str) > 0) {
        number_len = strlen(number_str);
        % Convert the string into integer to increment it
        number = atoll(number_str);
        number += (direction > 0) ? 1 : -1;
        new_number_buf = string(number);
        new_number_len = strlen(new_number_buf);

        % Move the cursor to the found number
        cpos = mc->cure_cursor_offset();
        mc->cure_cursor_move (pos-cpos);
        % Delete the existing number
        mc->cure_delete();
        for (idx = 0; idx < number_len-1; idx ++)
            mc->cure_backspace();
        % Insert updated number
        for (idx = 0; idx < new_number_len; idx ++)
            mc->cure_insert_ahead(new_number_buf[new_number_len-idx-1]);
    }

    % Return the updated number. Just for fun :) Maybe it'll find some use one day
    return number;
}

% The end user function for incrementing an integer
define grow_shrink_int__grow_int() {
    return grow_shrink_int__increment(1);
}

% The end user function for taking 1 from an integer
define grow_shrink_int__shrink_int() {
    return grow_shrink_int__increment(-1);
}

% Register the default key bindings – Alt-a for grow, Alt-x for shrink.
mc->editor_map_key_to_func("GrowInteger", "alt-a", "grow_shrink_int__grow_int");
mc->editor_map_key_to_func("ShrinkInteger", "alt-x", "grow_shrink_int__shrink_int");
