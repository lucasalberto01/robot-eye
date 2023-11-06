#ifndef CONTROL_H
#define CONTROL_H

class CamControl{
    public:
        CamControl();

        void setup();

        void setCamTilt(int angle);
        void setCamPan(int angle);

        void setCenter();
        void test();

    private:
        int checkLimit(int angle);

};


#endif  // CONTROL_H