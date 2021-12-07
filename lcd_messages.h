void introMessage(){
    write_str("   Teklados   ");
    go_to(1,0);
    write_str("   Mecanicos   ");
    _delay_ms(3000);
    clear_display();
    go_home();
}

void normalModeMessage(){
    write_str("      Modo     ");
    go_to(1,0);
    write_str("     Normal    ");
    _delay_ms(3000); 
    clear_display();
    go_home();
}

void configModeMessage(){
    clear_display();
    go_home();
    write_str("      Modo     ");
    go_to(1,0);
    write_str("  Configuracion ");
    _delay_ms(2000);
    clear_display();
    go_home();    
    write_str("    presiona   ");
    _delay_ms(1000);
    clear_display();
    go_home();
    write_str("       la      ");
    _delay_ms(1000);
    clear_display();
    go_home();
    write_str("     tecla     ");
    _delay_ms(1000);
    clear_display();
    go_home();
    write_str("       a       ");
    _delay_ms(1000);
    clear_display();
    go_home();
    write_str("   configurar    ");
    _delay_ms(1000);
    clear_display();
    go_home();
}

void updateCurrentConfig(char * key,unsigned int alphaIndex,char alpha[]){
    char  currentKey[10];
    sprintf(currentKey,"%s : ",key);
    go_home(); 
    write_str(currentKey);
    write_char(alpha[alphaIndex]);
}