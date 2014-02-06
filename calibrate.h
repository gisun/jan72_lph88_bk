void TS_calibrate(void){
    bgcolor = RGB(0x1E, 0x2E, 0x1E);
    s65_clear(bgcolor);

    s65_drawCircle( x, y, 15, RGB(0x1E, 0x00, 0x00));
    s65_drawLine(x-5, y, x+5, y, RGB(0x1E, 0x00, 0x00));
    s65_drawLine(x, y-5, x, y+5, RGB(0x1E, 0x00, 0x00));

    //clear
    sprintf( convert, "   ", x);
    s65_drawText( 78, 53, convert, 1, cl_21, BGCOLOR1);
    //print
    sprintf( convert, "%3.3d", x);
    s65_drawText( 78, 53, convert, 1, cl_21, BGCOLOR1);

    //clear
    sprintf( convert, "   ", x);
    s65_drawText( 78, 73, convert, 1, cl_21, BGCOLOR1);
    //print
    sprintf( convert, "%3.3d", y);
    s65_drawText( 78, 73, convert, 1, cl_21, BGCOLOR1);
}
