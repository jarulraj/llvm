
int main(){
    int r1, r2, r3;

    r1 = 1;
    r2 = 2;
    r3 = 3;

    if(r2){
        r1 = 5;
    }
    else{
        r1 = 6;
    }

    r1 = r2 + 1;
    r2 = r2 + 2;
    r3 = r2 + r3;
    //r1 = r2 + 3;

    return r3;
}
