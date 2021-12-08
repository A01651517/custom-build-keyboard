void introMessage(){
    write_str("   Teklados   ");
    go_to(1,0);
    write_str("   Mecanicos   ");
    wdt_reset();
    _delay_ms(1000);
    clear_display();
    go_home();
}

void normalModeMessage(){
    write_str("     Normal    ");
    go_to(1,0);
    write_str("      Mode     ");
    go_home();
}

void configModeMessage(){
    clear_display();
    go_home();
    write_str("     Config     ");
    go_to(1,0);
    write_str("      Mode     ");
    wdt_reset();
    _delay_ms(1000);
    wdt_reset();
    clear_display();
    go_home();    
    write_str(" Press any key ");
    go_to(1,0);
    write_str("or config 2 exit");
}

void updateCurrentConfig(char * key,unsigned int alphaIndex,char alpha[]){
    clear_display();
    go_home(); 
    char  currentKey[10];
    sprintf(currentKey,"%s : ",key);
    write_str(currentKey);
    write_char(alpha[alphaIndex]);
}
