# Controle de Servomotores por PWM — Raspberry Pi Pico RP2040

Aplicação embarcada desenvolvida com **Raspberry Pi Pico**, **Pico SDK** e **C/C++** para controle de servomotores utilizando o periférico de **PWM do RP2040**.

O projeto é dividido em três níveis progressivos. Cada etapa reaproveita a estrutura anterior e adiciona novos conceitos de sistemas embarcados:

1. controle automático de posição por temporização;
2. controle de posição por entrada analógica;
3. controle de dois servomotores utilizando um único potenciômetro.

---

## Integrantes

* **João Pedro de Jesus Cândido Silva** — R.A. 23.01416-4
* **Erich Abreu Serafim** — R.A. 23.10022-2

---

## Objetivo

Aplicar os periféricos de **PWM**, **ADC** e **timers** do RP2040 no controle de posição de servomotores.

A evolução do projeto segue a seguinte lógica:

```text
Nível 1
Sequência de ângulos
        ↓
      Timer
        ↓
      Ângulo
        ↓
       PWM
        ↓
      Servo


Nível 2
Potenciômetro
      ↓
     ADC
      ↓
   Ângulo
      ↓
     PWM
      ↓
    Servo


Nível 3
            Potenciômetro
                 ↓
                ADC
                 ↓
         Divisão da faixa
            ↙         ↘
       Servo 1       Servo 2
```

---

## Hardware utilizado

* Raspberry Pi Pico / RP2040
* Servomotor Tower Pro SG90
* Potenciômetro
* Protoboard
* Jumpers
* Cabo USB
* Fonte externa para o Nível 3, quando necessária

### GPIOs utilizados

| Função        |                GPIO |
| ------------- | ------------------: |
| Servo 1 — PWM |                 GP0 |
| Potenciômetro |         GP26 / ADC0 |
| Servo 2 — PWM | definido no Nível 3 |

---

## PWM do servomotor

Servomotores são controlados pela largura do pulso de um sinal PWM periódico.

A frequência utilizada no projeto é aproximadamente:

$$
f_{PWM} = 50\ \text{Hz}
$$

O período correspondente é:

$$
T = \frac{1}{f_{PWM}}
$$

Para $f_{PWM}=50\ \text{Hz}$:

$$
T = \frac{1}{50} = 0{,}02\ \text{s} = 20\ \text{ms}
$$

A referência inicial do laboratório era:

| Posição | Largura de pulso |
| ------: | ---------------: |
|      0° |           1,0 ms |
|     90° |           1,5 ms |
|    180° |           2,0 ms |

Durante os testes com os servomotores **SG90 utilizados na bancada**, essa faixa resultou em um deslocamento mecânico menor que 180°.

Após calibração experimental, foi adotada a faixa:

| Posição | Pulso utilizado |
| ------: | --------------: |
|      0° |          0,5 ms |
|     45° |          1,0 ms |
|     90° |          1,5 ms |
|    135° |          2,0 ms |
|    180° |          2,5 ms |

O mapeamento entre ângulo e largura de pulso é realizado por interpolação linear:

$$
\begin{aligned}
t_{\text{pulso}}
&= t_{\min}

* \frac{\theta-\theta_{\min}}
  {\theta_{\max}-\theta_{\min}}
  \left(t_{\max}-t_{\min}\right)
  \end{aligned}
  $$

Para os limites utilizados no projeto:

$$
\theta_{\min}=0^\circ
\qquad
\theta_{\max}=180^\circ
$$

$$
t_{\min}=0{,}5\ \text{ms}
\qquad
t_{\max}=2{,}5\ \text{ms}
$$

Portanto:

$$
\begin{aligned}
t_{\text{pulso}}
&= 0{,}5 + \frac{\theta}{180}(2{,}5-0{,}5)
\end{aligned}
$$

Como:

$$
2{,}5-0{,}5=2{,}0
$$

a expressão também pode ser escrita como:

$$
\begin{aligned}
t_{\text{pulso}}
&= 0{,}5 + \frac{\theta}{90}
\end{aligned}
$$

onde:

* $\theta$ é o ângulo desejado, em graus;
* $t_{\text{pulso}}$ é a largura do pulso, em milissegundos.

Por exemplo, para $\theta=90^\circ$:

$$
\begin{aligned}
t_{\text{pulso}}
&= 0{,}5 + \frac{90}{180}(2{,}0) \
&= 0{,}5 + 1{,}0 \
&= 1{,}5\ \text{ms}
\end{aligned}
$$

A calibração é mantida de forma centralizada no código, facilitando ajustes para outros servomotores.

---

## Configuração do PWM no RP2040

A configuração utilizada no projeto é:

```cpp
float fClkdiv = 125.0f;
vu16Wrap = 20000;
```

Considerando um clock do sistema de aproximadamente 125 MHz:

$$
f_{\text{PWM clock}}
====================

# \frac{125\ \text{MHz}}{125}

1\ \text{MHz}
$$

Assim, cada incremento do contador corresponde aproximadamente a:

$$
1\ \text{tick} = 1\ \mu s
$$

Com aproximadamente 20.000 contagens por período:

$$
T \approx 20000\ \mu s = 20\ \text{ms}
$$

e, consequentemente:

$$
f_{PWM} \approx 50\ \text{Hz}
$$

Essa configuração facilita a interpretação da largura dos pulsos aplicados ao servomotor.

---

## Seleção do nível

As implementações são mantidas no mesmo arquivo e selecionadas através de uma flag definida no início do código:

```cpp
#define LEVEL 1
```

ou:

```cpp
#define LEVEL 2
```

e, para a etapa final:

```cpp
#define LEVEL 3
```

Dessa forma, cada nível pode ser compilado e testado separadamente sem remover as implementações anteriores.

---

## Nível 1 — Sequência automática de posições

No primeiro nível, um único servomotor é controlado automaticamente através da seguinte sequência:

```text
0°
↓
45°
↓
90°
↓
135°
↓
180°
↓
135°
↓
90°
↓
45°
↓
0°
```

A sequência utilizada no código é:

```cpp
float sequence[9] = {
    0.0f,
    45.0f,
    90.0f,
    135.0f,
    180.0f,
    135.0f,
    90.0f,
    45.0f,
    0.0f
};
```

Cada posição é mantida por aproximadamente **2 segundos**.

A temporização é realizada através de um `repeating_timer` do Pico SDK:

```cpp
add_repeating_timer_ms(2000, timer1_cb, NULL, &timer1);
```

A callback altera apenas a referência angular do servo, enquanto o periférico PWM continua gerando o sinal continuamente em aproximadamente 50 Hz.

O fluxo é:

```text
Timer de 2 s
     ↓
sequence[cont]
     ↓
set_servo_angle()
     ↓
ângulo → pulso
     ↓
duty cycle
     ↓
PWM
     ↓
servo
```

---

## Nível 2 — Controle por potenciômetro

No segundo nível, a sequência automática é substituída pelo controle contínuo através de um potenciômetro conectado ao:

```text
GP26 / ADC0
```

O ADC do RP2040 possui resolução de 12 bits, fornecendo valores entre:

```text
0 ... 4095
```

A leitura é inicialmente normalizada:

$$
p = \frac{ADC}{4095}
$$

onde $p$ varia entre 0 e 1.

Em seguida, o percentual é convertido em ângulo:

$$
\theta = 180p
$$

Portanto:

|  ADC | Percentual | Ângulo aproximado |
| ---: | ---------: | ----------------: |
|    0 |         0% |                0° |
| 1024 |        25% |               45° |
| 2048 |        50% |               90° |
| 3071 |        75% |              135° |
| 4095 |       100% |              180° |

O fluxo completo é:

```text
Potenciômetro
     ↓
GPIO26 / ADC0
     ↓
 adc_read()
     ↓
 0 ... 4095
     ↓
 normalização
     ↓
 0 ... 180°
     ↓
set_servo_angle()
     ↓
0,5 ... 2,5 ms
     ↓
 PWM 50 Hz
     ↓
   SG90
```

### Estabilidade da leitura analógica

Leituras analógicas apresentam pequenas variações mesmo quando o potenciômetro está parado.

Sem tratamento, essas pequenas oscilações poderiam causar atualizações constantes da posição do servomotor e gerar tremores.

Para reduzir esse efeito, a implementação do Nível 2 utiliza:

* média de múltiplas leituras do ADC;
* pequena deadband angular;
* atualização periódica do controle.

A média é realizada sobre múltiplas amostras:

```cpp
#define ADC_NUM_SAMPLES 8
```

e a deadband angular é definida por:

```cpp
#define ANGLE_DEADBAND 1.0f
```

Assim, pequenas oscilações que não representam um movimento real do potenciômetro são ignoradas.

---

## Nível 3 — Dois servomotores com um potenciômetro

No terceiro nível, a arquitetura do Nível 2 é expandida para duas saídas PWM independentes.

A faixa total do ADC é dividida em duas regiões:

```text
ADC

0 -------------------- 2048 -------------------- 4095
|                        |                         |
|                        |                         |
└──── Servo 1 ───────────┘└──── Servo 2 ─────────┘
      0° → 180°                 0° → 180°
```

Na primeira metade:

$$
0 \le ADC \le 2048
$$

o Servo 1 percorre de 0° a 180°.

Na segunda metade:

$$
2048 < ADC \le 4095
$$

o Servo 2 percorre de 0° a 180°.

Cada servo deve utilizar uma saída PWM própria, permitindo o controle independente de posição.

O comportamento do servo que estiver fora de sua região ativa deve ser definido explicitamente no código. Exemplos possíveis:

* manter a última posição;
* permanecer em 0°;
* permanecer em 180°.

> **Status:** Nível 3 em desenvolvimento.

---

## Montagem elétrica

### Níveis 1 e 2

Para um único servomotor:

```text
Raspberry Pi Pico            SG90
-----------------            ----
GPIO0 ---------------------> Signal
GND   ---------------------> GND
VBUS  ---------------------> VCC
```

Para o potenciômetro:

```text
Pico 3V3  -------- extremo do potenciômetro
Pico GND  -------- extremo oposto
GPIO26    -------- terminal central / cursor
```

O potenciômetro funciona como um divisor de tensão entre 0 V e 3,3 V.

O terminal central fornece ao ADC uma tensão proporcional à posição do potenciômetro.

### Nível 3

Com dois servomotores, recomenda-se utilizar uma fonte externa adequada.

Exemplo:

```text
Fonte +  -------- Servo 1 VCC
          └------ Servo 2 VCC

Fonte GND ------- Servo 1 GND
          ├------ Servo 2 GND
          └------ Pico GND
```

É essencial manter **GND comum** entre:

* Raspberry Pi Pico;
* fonte externa;
* Servo 1;
* Servo 2.

> Os servomotores não devem ser alimentados pelo pino 3V3 do Raspberry Pi Pico.

Também deve ser evitada a ligação da saída positiva da fonte externa diretamente em paralelo com o VBUS/USB do Pico.

---

## Estrutura do projeto

Uma organização típica do repositório é:

```text
Controle-Servomotores-PWM/
├── CMakeLists.txt
├── main.cpp
├── pico_sdk_import.cmake
├── README.md
└── videos/
```

As diferentes implementações são selecionadas no próprio código através da macro `LEVEL`.

---

## Compilação

O projeto utiliza o **Raspberry Pi Pico SDK**.

Com o ambiente corretamente configurado:

```bash
mkdir build
cd build
cmake ..
make -j
```

Após a compilação, o arquivo `.uf2` gerado pode ser gravado no Raspberry Pi Pico utilizando o modo BOOTSEL.

Também é possível compilar e carregar o projeto diretamente através da extensão oficial do Raspberry Pi Pico para Visual Studio Code.

---

## Monitoramento serial

Durante a execução são enviados valores pela interface serial para auxiliar na validação e depuração.

No Nível 1, por exemplo:

```text
angle=0, pulse=0.500 ms, duty=0.0250, pwm=500
angle=45, pulse=1.000 ms, duty=0.0500, pwm=1000
angle=90, pulse=1.500 ms, duty=0.0750, pwm=1500
angle=135, pulse=2.000 ms, duty=0.1000, pwm=2000
angle=180, pulse=2.500 ms, duty=0.1250, pwm=2500
```

No Nível 2 também podem ser monitorados:

```text
ADC
Percentual
Ângulo calculado
Largura do pulso
Duty cycle
PWM level
```

Isso permite verificar cada etapa da cadeia de controle separadamente.

---

## Conceitos aplicados

O projeto trabalha diretamente com os seguintes conceitos de sistemas embarcados:

* PWM;
* frequência e período;
* duty cycle;
* largura de pulso;
* timers;
* callbacks;
* ADC;
* aquisição analógica;
* mapeamento linear;
* interpolação;
* calibração de atuadores;
* filtragem simples;
* deadband;
* controle de periféricos do RP2040.

---

## Requisitos atendidos

* [x] Controle de servomotor através do hardware PWM do RP2040
* [x] PWM de aproximadamente 50 Hz
* [x] Conversão centralizada entre ângulo e largura de pulso
* [x] Limitação da faixa de atuação do servomotor
* [x] Nível 1 com sequência automática
* [x] Nível 1 utilizando timer do Pico SDK
* [x] Intervalo de aproximadamente 2 segundos entre posições
* [x] ADC utilizando GP26 / ADC0
* [x] Mapeamento ADC → 0°–180°
* [x] Controle contínuo por potenciômetro
* [x] Tratamento de pequenas oscilações do ADC
* [ ] Controle de dois servomotores — Nível 3

---

## Calibração do servomotor

Durante os testes foi identificada uma diferença entre a faixa de pulsos utilizada inicialmente e a necessária para obter aproximadamente 180° de deslocamento nos servomotores SG90 utilizados.

A calibração experimental adotada foi:

```text
500 µs  → 0°
1500 µs → 90°
2500 µs → 180°
```

Essa calibração é mantida no código através das constantes de limite do servomotor.

Caso outro servomotor apresente comportamento diferente, os limites podem ser ajustados sem alterar a lógica geral do controle.

---

## Diagnóstico de problemas

Caso o servomotor apresente:

* tremores excessivos;
* ruídos mecânicos;
* perda de posição;
* movimentos inesperados;
* resets do Raspberry Pi Pico;

recomenda-se verificar:

1. alimentação do servomotor;
2. conexão de GND comum;
3. limites de largura de pulso;
4. montagem mecânica;
5. estabilidade da leitura ADC;
6. conexões entre GPIO e sinal do servo.

Se o servo atingir um limite mecânico e continuar tentando se mover, a alimentação deve ser interrompida e os limites de pulso devem ser revisados.

---

## Repositório

https://github.com/Jujupedro5052005/Controle-Servomotores-PWM

---

## Autores

**João Pedro de Jesus Cândido Silva**
R.A. 23.01416-4

**Erich Abreu Serafim**
R.A. 23.10022-2
