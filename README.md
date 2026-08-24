# Controle de Servomotores por PWM — Raspberry Pi Pico RP2040

Aplicação embarcada desenvolvida com **Raspberry Pi Pico**, **Pico SDK** e **C/C++** para estudo e implementação do controle de servomotores utilizando o periférico de **PWM do RP2040**.

O projeto é dividido em três níveis progressivos. Cada etapa reutiliza a estrutura desenvolvida anteriormente e adiciona novos conceitos de sistemas embarcados:

1. controle automático de posição por temporização;
2. controle de posição por entrada analógica;
3. controle de dois servomotores utilizando um único potenciômetro.

---

## Objetivo

Aplicar os periféricos de **PWM**, **ADC** e **timers** do RP2040 no controle de posição de servomotores.

A evolução do projeto segue a arquitetura:

```text
Nível 1
Sequência de ângulos
        ↓
      Timer
        ↓
Ângulo → PWM → Servo


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

```

---

# Hardware utilizado

* Raspberry Pi Pico / RP2040
* Servomotor Tower Pro SG90
* Potenciômetro
* Protoboard
* Jumpers
* Cabo USB
* Fonte externa para a implementação com dois servomotores, quando necessária

### GPIOs utilizados

| Função        |                GPIO |
| ------------- | ------------------: |
| Servo 1 — PWM |                 GP0 |
| Potenciômetro |         GP26 / ADC0 |
| Servo 2 — PWM | definido no Nível 3 |

---

# PWM do servomotor

Servomotores são controlados pela largura do pulso de um sinal PWM periódico.

A frequência utilizada no projeto é aproximadamente:

[
f_{PWM}=50\ Hz
]

correspondente a um período de aproximadamente:

[
T=\frac{1}{50}=20\ ms
]

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

Assim, o mapeamento utilizado no projeto é:

[
t_{pulso}
=========

0,5+
\frac{\theta}{180}(2,5-0,5)
]

onde:

* (\theta) é o ângulo desejado em graus;
* (t_{pulso}) é a largura do pulso em milissegundos.

A calibração foi mantida concentrada em constantes e na função responsável pela conversão de ângulo para PWM, facilitando alterações futuras.

---

# Configuração do PWM no RP2040

A implementação utiliza:

```cpp
float fClkdiv = 125.0f;
vu16Wrap = 20000;
```

Considerando o clock de aproximadamente 125 MHz do RP2040:

```text
125 MHz / 125 = 1 MHz
```

Portanto, o contador PWM opera com aproximadamente:

```text
1 tick = 1 µs
```

e:

```text
20000 ticks ≈ 20 ms ≈ 50 Hz
```

Essa configuração também facilita a interpretação dos valores de comparação utilizados pelo PWM.

---

# Seleção do nível

As implementações são mantidas no mesmo código e selecionadas por uma flag de compilação no início do arquivo:

```cpp
#define LEVEL 1
```

ou:

```cpp
#define LEVEL 2
```

e futuramente:

```cpp
#define LEVEL 3
```

Isso permite testar cada etapa separadamente sem remover a implementação dos níveis anteriores.

---

# Nível 1 — Sequência automática de posições

O primeiro nível controla um único servomotor através de uma sequência predefinida:

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

Cada posição é mantida por aproximadamente **2 segundos**.

A sequência é armazenada em um vetor:

```cpp
float sequence[9] = {
    0.0f, 45.0f, 90.0f, 135.0f, 180.0f,
    135.0f, 90.0f, 45.0f, 0.0f
};
```

A atualização não utiliza `sleep_ms()` como mecanismo principal de controle.

Foi utilizado um **repeating timer do Pico SDK**:

```cpp
add_repeating_timer_ms(2000, timer1_cb, NULL, &timer1);
```

A cada chamada da callback, o próximo ângulo da sequência é enviado ao servomotor.

## Demonstração — Nível 1

![Demonstração do Nível 1](videos/level1.gif)

---

# Nível 2 — Controle por potenciômetro

No segundo nível, a sequência automática é substituída pela leitura contínua de um potenciômetro conectado a:

```text
GP26 / ADC0
```

O ADC do RP2040 possui resolução de 12 bits, fornecendo valores entre:

```text
0 ... 4095
```

A leitura é convertida para um percentual:

[
p=\frac{ADC}{4095}
]

e posteriormente para um ângulo:

[
\theta = 180p
]

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

## Estabilidade da leitura

Pequenas variações da leitura analógica podem resultar em pequenas correções contínuas de posição e, consequentemente, tremores no servomotor.

Para reduzir esse efeito, a implementação pode utilizar:

* média de múltiplas leituras do ADC;
* deadband angular;
* atualização periódica através de timer.

A solução implementada utiliza média de amostras e uma pequena deadband antes de atualizar a posição.

## Demonstração — Nível 2

![Demonstração do Nível 2](videos/level2.gif)

---

# Nível 3 — Dois servomotores com um potenciômetro

O terceiro nível expande a arquitetura do Nível 2 para duas saídas PWM independentes.

A faixa do potenciômetro é dividida em duas regiões:

```text
ADC
0 -------------------------------------------- 4095
|                     |
|                     |
0                   ~2048                   4095

      Servo 1                 Servo 2
      0 → 180°                0 → 180°
```

Na primeira metade:

[
0 \le ADC \le ADC_{mid}
]

o **Servo 1** percorre de 0° a 180°.

Na segunda metade:

[
ADC_{mid} < ADC \le 4095
]

o **Servo 2** percorre de 0° a 180°.

O comportamento do servo fora de sua região ativa deve ser definido explicitamente pela implementação, por exemplo:

* permanecer no último ângulo;
* permanecer em 0°;
* permanecer em 180°.

> **Status:** implementação do Nível 3 em desenvolvimento.

---

# Montagem elétrica

## Níveis 1 e 2

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
3V3  -------- extremo do potenciômetro
GND  -------- extremo oposto
GP26 -------- terminal central / cursor
```

O potenciômetro funciona como um divisor de tensão entre **0 V e 3,3 V**, compatível com a entrada ADC do RP2040.

## Nível 3

Com dois servomotores, recomenda-se alimentação externa adequada.

Nesse caso:

```text
Fonte +  -------- Servo 1 VCC
          └------ Servo 2 VCC

Fonte GND ------- Servo 1 GND
          ├------ Servo 2 GND
          └------ Pico GND
```

O **GND deve ser comum** entre:

* Raspberry Pi Pico;
* fonte externa;
* Servo 1;
* Servo 2.

> Não alimentar os servomotores através do pino `3V3` do Raspberry Pi Pico.

---

# Estrutura do projeto

Uma organização típica do repositório é:

```text
Controle-Servomotores-PWM/
├── CMakeLists.txt
├── main.cpp
├── pico_sdk_import.cmake
├── README.md
└── videos/
    ├── level1.gif
    └── level2.gif
```

---

# Compilação

O projeto utiliza o **Raspberry Pi Pico SDK**.

Com o ambiente do Pico SDK corretamente configurado:

```bash
mkdir build
cd build
cmake ..
make -j
```

Após a compilação, o arquivo `.uf2` pode ser copiado para o Raspberry Pi Pico em modo **BOOTSEL**.

Em sistemas utilizando a extensão oficial do Raspberry Pi Pico no VS Code, o projeto também pode ser compilado e carregado diretamente pelas ferramentas fornecidas pela extensão.

---

# Monitoramento serial

Durante o desenvolvimento, são enviados pela interface serial valores úteis para depuração.

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

Isso permite verificar separadamente cada etapa da cadeia de controle.

---

# Conceitos aplicados

O projeto explora diretamente os seguintes conceitos de sistemas embarcados:

* PWM;
* frequência e período;
* duty cycle;
* largura de pulso;
* timers;
* callbacks;
* ADC;
* aquisição analógica;
* mapeamento linear;
* calibração de atuadores;
* filtragem simples;
* deadband;
* controle de múltiplos periféricos do RP2040.

---

# Requisitos atendidos

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

# Observações

Durante os testes foi identificada diferença entre a faixa nominal de pulsos indicada inicialmente e a faixa necessária para obter aproximadamente 180° de deslocamento nos SG90 utilizados.

A calibração experimental adotada foi:

```text
500 µs  → 0°
1500 µs → 90°
2500 µs → 180°
```

Esses valores estão concentrados no código para permitir ajuste simples caso outro servomotor apresente limites mecânicos diferentes.

Se o servo apresentar:

* tremores excessivos;
* ruídos mecânicos;
* perda de posição;
* resets do Raspberry Pi Pico;

devem ser verificados, nesta ordem:

1. alimentação do servomotor;
2. conexão de GND comum;
3. limites de largura de pulso;
4. montagem mecânica;
5. estabilidade da leitura ADC.

---

## Repositório

[github.com/Jujupedro5052005/Controle-Servomotores-PWM](https://github.com/Jujupedro5052005/Controle-Servomotores-PWM)

---

## Autores

**João Pedro de Jesus Cândido Silva**
R.A. 23.01416-4

**Erich Abreu Serafim**
R.A. 23.10022-2
