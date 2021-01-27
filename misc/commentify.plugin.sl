% Copy to ~/.config/mc/plugin for this file to be loaded at mc's startup.

define commentify()
{
    variable cur_pos;   % Current cursor offset
    variable bol;       % Begin-Of-Line offset

    % Get current cursor position and beginning of current line.
    cur_pos = mc->cure_cursor_offset();
    bol = mc->cure_get_bol();
    % Move to the start of the line.
    mc->cure_cursor_move(bol - cur_pos);
    % Insert "/*".
    mc->cure_insert_ahead('*');
    mc->cure_insert_ahead('/');
    % Move to the end of the line.
    mc->cure_cursor_move(mc->cure_get_eol()-bol);
    % Insert "*/".
    mc->cure_insert_ahead('/');
    mc->cure_insert_ahead('*');
}

mc->editor_map_key_to_func("Commentify","alt-i","commentify");
