void introMessage(){
    write_str((unsigned char *)"   Teklados   ");
    go_to(1,0);
    write_str((unsigned char *)"   Mecanicos   ");
    wdt_reset();
    _delay_ms(1000);
    clear_display();
    go_home();
}

void normalModeMessage(){
    write_str((unsigned char *)"     Normal    ");
    go_to(1,0);
    write_str((unsigned char *)"      Mode     ");
    go_home();
}

void configModeMessage(){
    clear_display();
    go_home();
    write_str((unsigned char *)"     Config     ");
    go_to(1,0);
    write_str((unsigned char *)"      Mode     ");
    wdt_reset();
    _delay_ms(1000);
    wdt_reset();
    clear_display();
    go_home();    
    write_str((unsigned char *)" Press any key ");
    go_to(1,0);
    write_str((unsigned char *)"or config 2 exit");
}

void updateCurrentConfig(char * key,unsigned int alphaIndex, const char alpha[]){
    clear_display();
    go_home(); 
    char  currentKey[10];
    sprintf(currentKey,"%s : ",key);
    write_str((unsigned char *)currentKey);
    write_char(alpha[alphaIndex]);
}
