#include <avr/io.h>

#ifndef __IO_H__
#define __IO_H__

namespace avr
{
    // a memory mapped byte represention an I/O register
    typedef volatile uint8_t *MMIO;

    class Port
    {
    public:
        Port(MMIO ddr_reg, MMIO port_reg, MMIO pin_reg) : ddr_reg(ddr_reg), out_reg(port_reg), in_reg(pin_reg) {}
        Port() = delete;
        MMIO ddr_reg;
        MMIO out_reg;
        MMIO in_reg;
    };

    class Pin
    {
    public:
        Pin(const Port &port, const uint8_t pin_no, const bool activeLow)
            : port(port), pin_no(pin_no), activeLow(activeLow) {}
        Pin() = delete;

        const Port &port;
        const uint8_t pin_no;
        const bool activeLow;
    };

    class Output : public Pin
    {
    public:
        Output(const Port &port, uint8_t pin_no, bool activeLow = false) : Pin(port, pin_no, activeLow)
        {
            *(this->port.ddr_reg) |= (1 << this->pin_no);
        }

        const bool &operator<<(bool const &level)
        {
            if (level ^ activeLow)
            {
                *(this->port.out_reg) |= (1 << this->pin_no);
            }
            else
            {
                *(this->port.out_reg) &= ~(1 << this->pin_no);
            }
            return level;
        }
    };

    class TristateOutput : public Output
    {
    public:
        enum class DriveLevel
        {
            LOW = 0,
            HIGH = 1,
            HIGH_Z = 2
        };

        TristateOutput(const Port &port, uint8_t pin_no, bool activeLow = false) : Output(port, pin_no, activeLow)
        {
            *(this->port.ddr_reg) |= (1 << this->pin_no);
        }

        const DriveLevel &operator<<(DriveLevel const &level)
        {
            bool levelVal;
            switch (level)
            {
            case DriveLevel::HIGH:
            case DriveLevel::LOW:
                *this->port.ddr_reg |= (1 << this->pin_no);
                levelVal = (bool)level;
                break;
            case DriveLevel::HIGH_Z:
                *this->port.ddr_reg &= ~(1 << this->pin_no);
                levelVal = false;
                break;
            }

            Output::operator<<(levelVal);

            return level;
        }
    };

    class Input : public Pin
    {
    public:
        Input(const Port &port, uint8_t pin_no, bool activeLow = false, bool pullupEnabled = false) : Pin(port, pin_no, activeLow)
        {
            *(this->port.ddr_reg) |= (1 << this->pin_no);
            if (pullupEnabled)
            {
                *(this->port.out_reg) |= (1 << this->pin_no);
            }
        }

        operator bool() const
        {
            return ((*this->port.in_reg) & (1 << this->pin_no)) ^ activeLow;
        }
    };

#ifdef PORTA
    avr::Port PortA(&DDRA, &PORTA, &PINA);
#endif

#ifdef PORTB
    avr::Port PortB(&DDRB, &PORTB, &PINB);
#endif

#ifdef PORTC
    avr::Port PortC(&DDRC, &PORTC, &PINC);
#endif

#ifdef PORTD
    avr::Port PortD(&DDRD, &PORTD, &PIND);
#endif

} // namespace avr

#endif // __IO_H__
