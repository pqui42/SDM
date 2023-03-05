#ifndef SAFALO_H
#define SAFALO_H

class SafAlo {

    public: 
        static SafAlo & Get(){
            static SafAlo instance;
            return instance;
        }

        void AloPermit() {
            alo_alow_ = true;
        }

        void AloProhibit() {
            alo_alow_ = false;
        }

        bool IsAloAllowed() {
            return alo_alow_;
        }

    private:
        SafAlo() : alo_alow_{true} {}
        ~SafAlo() {};

        bool alo_alow_;
};

#endif // SAFALO_H