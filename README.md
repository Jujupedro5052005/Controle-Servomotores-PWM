Controle de Servomotores com Raspberry Pi Pico e RP2040

Projeto desenvolvido em C utilizando o Raspberry Pi Pico SDK para controle de servomotores por meio das saídas de PWM do RP2040. O desafio é dividido em três níveis progressivos, explorando geração de PWM, temporização, leitura analógica com ADC e controle de múltiplos servomotores.

🎯 Objetivo

Aplicar a geração de PWM do RP2040 no controle de posição de servomotores, evoluindo progressivamente:

Nível 1: sequência automática de posições;
Nível 2: controle de um servomotor por potenciômetro;
Nível 3: controle de dois servomotores utilizando um único potenciômetro.
🛠️ Tecnologias utilizadas
Raspberry Pi Pico
RP2040
Pico SDK
Linguagem C
CMake
PWM
ADC
Servomotores
Potenciômetro
📐 Referência de PWM

Para os servomotores utilizados no laboratório, foi adotada como referência uma frequência de aproximadamente 50 Hz, correspondente a um período de aproximadamente 20 ms.

Posição	Largura do pulso
0°	~1,0 ms
45°	~1,125 ms
90°	~1,5 ms
135°	~1,75 ms
180°	~2,0 ms

Observação: os valores acima são referências de laboratório. Caso o servomotor atinja seus limites mecânicos antes dos extremos, a faixa de pulsos deve ser reduzida e os limites adotados devem ser registrados no código.

🚀 Níveis do Desafio
Nível 1 — Sequência automática de posições

O primeiro nível consiste no controle de um servomotor utilizando uma saída PWM do RP2040.

A sequência executada é:

0° → 45° → 90° → 135° → 180°
                    ↓
0° ← 45° ← 90° ← 135°


Ou, de forma linear:

0° → 45° → 90° → 135° → 180° → 135° → 90° → 45° → 0°

Requisitos
Controlar um servomotor por PWM;
Utilizar aproximadamente 50 Hz;
Executar a sequência de posições;
Aguardar aproximadamente 2 segundos entre cada mudança;
Utilizar um recurso de timer do Pico SDK para controlar a temporização;
Não utilizar sleep_ms() como mecanismo principal de controle da sequência;
Atualizar o duty cycle/nível ativo do PWM a cada mudança de posição.
Estratégia utilizada

A sequência de ângulos pode ser armazenada em um vetor:

const int angles[] = {
    0, 45, 90, 135, 180, 135, 90, 45, 0
};


Um índice é utilizado para percorrer os valores e atualizar o PWM periodicamente.

🎛️ Nível 2 — Controle por potenciômetro

No segundo nível, a sequência automática é substituída pelo controle contínuo através de uma entrada analógica.

O potenciômetro é conectado ao:

GP26 / ADC0

Funcionamento

O valor do ADC é lido continuamente e convertido para uma posição angular entre 0° e 180°.

ADC mínimo                         ADC máximo
    │                                  │
    ▼                                  ▼
   0° ───────────► ... ───────────► 180°

Requisitos
Utilizar o ADC0 / GP26;
Realizar leituras com adc_read();
Mapear a leitura do ADC para uma faixa de 0° a 180°;
Atualizar continuamente a posição do servomotor;
Manter o sinal PWM em aproximadamente 50 Hz;
Evitar comandos fora dos limites definidos para o servomotor.

A utilização de interrupção do ADC não é obrigatória.

🎚️ Nível 3 — Dois servomotores com um potenciômetro

No terceiro nível, são utilizados:

1 potenciômetro;
2 saídas PWM independentes;
2 servomotores.

A mesma leitura do potenciômetro é dividida em duas regiões.

ADC
0%                50%                100%
│------------------│-------------------│
│                  │                   │
│     SERVO 1      │      SERVO 2      │
│      0 → 180°    │       0 → 180°    │
│                  │                   │
└──────────────────┴───────────────────┘

Região do Servo 1

Na primeira metade da faixa do ADC:

ADC mínimo → ponto médio
      ↓
Servo 1: 0° → 180°

Região do Servo 2

Na segunda metade da faixa do ADC:

ponto médio → ADC máximo
      ↓
Servo 2: 0° → 180°

Comportamento fora da região ativa

Foi adotada a seguinte convenção:

O servomotor que estiver fora da região ativa mantém sua última posição.

Essa decisão evita movimentos desnecessários e torna o comportamento previsível durante a transição entre as duas regiões.

Essa convenção está documentada no código e pode ser alterada caso o projeto adote outro comportamento.

🔌 Conexões
Níveis 1 e 2

Para um único servomotor, a montagem pode utilizar a alimentação disponível no laboratório conforme orientação da atividade.

Exemplo
Raspberry Pi Pico
┌─────────────────────┐
│                     │
│ GPIO PWM ───────────────► Sinal do Servo
│                     │
│ VBUS ────────────────► VCC do Servo
│                     │
│ GND ────────────────► GND do Servo
│                     │
│ GP26 / ADC0 ◄────────── Potenciômetro
│                     │
└─────────────────────┘

Nível 3

Para dois servomotores, é recomendada a utilização de uma fonte externa de bancada.

Regra importante

O GND deve ser comum entre:

Raspberry Pi Pico;
fonte externa;
servo 1;
servo 2.
             Fonte externa
             ┌───────────┐
             │           │
             │   +V ───────────► VCC dos servos
             │           │
             │  GND ───────────► GND dos servos
             └─────┬─────┘
                   │
                   │ GND comum
                   ▼
              Raspberry Pi Pico

⚠️ Cuidados
Não alimentar os servomotores pelo pino 3V3 do Pico.
Ao utilizar fonte externa, conectar todos os GNDs em comum.
Evitar colocar a saída positiva da fonte em paralelo com o VBUS/USB do Pico.
Em caso de tremores, resets ou comportamento instável, verificar primeiro:
alimentação;
GND comum;
limites de pulso;
conexões elétricas.
📌 GPIOs

Os GPIOs utilizados devem estar claramente identificados no código.

Exemplo de organização:

// PWM
#define SERVO1_GPIO 15
#define SERVO2_GPIO 16

// ADC
#define POT_ADC_GPIO 26
#define POT_ADC_CHANNEL 0


Os GPIOs de PWM podem ser alterados conforme a montagem utilizada. O GP26/ADC0 deve ser utilizado para o potenciômetro, conforme especificação da atividade.

⚙️ Conversão de ângulo para PWM

A conversão entre ângulo e largura de pulso deve ficar concentrada em uma função específica, evitando números mágicos espalhados pelo código.

Exemplo conceitual:

uint16_t angle_to_pulse_us(uint16_t angle)
{
    angle = clamp(angle, 0, 180);

    return MIN_PULSE_US +
           ((MAX_PULSE_US - MIN_PULSE_US) * angle) / 180;
}


Os limites devem ser definidos por constantes:

#define SERVO_MIN_PULSE_US 1000
#define SERVO_MAX_PULSE_US 2000
#define SERVO_FREQUENCY_HZ 50


Dessa forma, a calibração do projeto fica centralizada e fácil de modificar.

📊 Estabilidade da leitura analógica

Pequenas oscilações do potenciômetro podem provocar pequenos movimentos do servomotor.

Para reduzir esse efeito, podem ser utilizadas técnicas como:

Deadband

Ignorar pequenas variações:

Se |novo_angulo - ultimo_angulo| < DEAD_BAND
    não atualizar o servo

Média de leituras

Realizar várias leituras e calcular a média:

ADC1
ADC2
ADC3
...
ADC10
 ↓
Média
 ↓
Ângulo
 ↓
PWM


Essas técnicas são opcionais, mas ajudam a melhorar a estabilidade.

🧮 Mapeamento do ADC

Considerando um ADC de 12 bits:

ADC_MIN = 0
ADC_MAX = 4095


Para o Nível 2:

0 ─────────────────────────── 4095
│                               │
0°                              180°


Uma conversão possível é:

angle = (adc_value * 180) / 4095;


Para o Nível 3:

0 ─────────────── 2047 ─────────────── 4095
│                    │                    │
│     Servo 1        │       Servo 2      │
│      0 → 180°      │        0 → 180°    │


O ponto médio do ADC deve ser definido claramente no código.

📁 Organização do projeto

Uma possível estrutura para o repositório é:

.
├── CMakeLists.txt
├── README.md
├── pico_sdk_import.cmake
├── src/
│   ├── nivel1/
│   │   └── main.c
│   ├── nivel2/
│   │   └── main.c
│   └── nivel3/
│       └── main.c
└── build/


Outra possibilidade é manter cada nível como uma versão independente do mesmo projeto.

O importante é que os três níveis estejam claramente identificados e sejam compiláveis.

🔨 Compilação

Com o Pico SDK configurado, o projeto pode ser compilado utilizando CMake.

Exemplo:

mkdir build
cd build
cmake ..
make -j4


Após a compilação, será gerado o arquivo .uf2 correspondente ao projeto.

O arquivo .uf2 pode ser transferido para o Raspberry Pi Pico utilizando o procedimento padrão de gravação.

🧪 Testes
Nível 1

Verificar:

 PWM próximo de 50 Hz;
 Servo inicia em 0°;
 Servo percorre todas as posições especificadas;
 Intervalo de aproximadamente 2 segundos entre posições;
 Sequência é executada corretamente;
 Temporização utiliza recurso de timer do Pico SDK.
Nível 2

Verificar:

 Potenciômetro conectado ao GP26/ADC0;
 ADC inicializado corretamente;
 Leitura contínua funcionando;
 ADC mínimo corresponde aproximadamente a 0°;
 ADC máximo corresponde aproximadamente a 180°;
 Servo acompanha o potenciômetro;
 Movimento permanece estável.
Nível 3

Verificar:

 Dois PWM independentes configurados;
 Dois servomotores conectados corretamente;
 Potenciômetro conectado ao GP26/ADC0;
 Primeira metade do ADC controla o Servo 1;
 Segunda metade controla o Servo 2;
 Servo fora da região ativa mantém a última posição;
 Fonte externa utilizada adequadamente;
 GND comum entre Pico e fonte.
📏 Calibração

Os valores abaixo podem ser ajustados conforme o servomotor utilizado:

#define SERVO_MIN_PULSE_US 1000
#define SERVO_CENTER_PULSE_US 1500
#define SERVO_MAX_PULSE_US 2000


Caso seja observado comportamento mecânico anormal, deve-se interromper o acionamento e verificar os limites.

Nunca forçar mecanicamente o servomotor contra seus limites.

📝 Depuração

Durante o desenvolvimento, é possível utilizar a interface serial para acompanhar:

ADC: 2048
Ângulo: 90°
Pulso: 1500 us


Essas informações facilitam a identificação de problemas na conversão ADC → ângulo → PWM.

Também é recomendado, quando houver disponibilidade, utilizar um osciloscópio ou analisador lógico para verificar:

período próximo de 20 ms;
frequência próxima de 50 Hz;
largura dos pulsos;
comportamento durante a alteração da posição.
📋 Critérios de avaliação

O projeto deve atender aos seguintes pontos:

Configuração correta da frequência e dos parâmetros do PWM;
Conversão correta entre ângulo e largura de pulso/duty cycle;
Execução correta da sequência do Nível 1;
Intervalo de aproximadamente 2 segundos no Nível 1;
Uso adequado do mecanismo de temporização;
Inicialização e leitura correta do ADC no Nível 2;
Mapeamento coerente do potenciômetro para 0°–180°;
Resposta contínua e estável no Nível 2;
Configuração de duas saídas PWM independentes no Nível 3;
Divisão correta da faixa do potenciômetro;
Convenção coerente para o servomotor fora de sua região ativa;
Montagem elétrica adequada;
Alimentação correta dos servomotores;
GND comum quando utilizada fonte externa;
Código organizado, legível e robusto.
👥 Integrantes

Adicionar no início de cada main.c:

/*
 * Projeto: Controle de Servomotores - Raspberry Pi Pico
 *
 * Integrantes:
 * Nome Completo - RA XXXXXXXX
 * Nome Completo - RA XXXXXXXX
 * Nome Completo - RA XXXXXXXX
 */

📦 Entrega

A entrega deve conter:

 Implementação do Nível 1;
 Implementação do Nível 2;
 Implementação do Nível 3;
 Arquivos necessários para compilação com Pico SDK/CMake;
 Nome completo e RA de todos os integrantes no código;
 Código funcional e compilável;
 Níveis claramente identificados;
 Projeto compactado em um único arquivo .zip.
⭐ Funcionalidades opcionais

As seguintes melhorias podem ser implementadas:

Média móvel do ADC;
Deadband para redução de tremores;
Calibração individual dos pulsos mínimo, central e máximo;
Saída de informações pela interface serial;
Medição do PWM com osciloscópio;
Medição do PWM com analisador lógico;
Organização do código em funções e módulos reutilizáveis.
⚠️ Segurança e cuidados

Antes de realizar os testes:

Confira todas as conexões;
Verifique a polaridade da alimentação;
Garanta que o GND seja comum quando houver fonte externa;
Não utilize o pino 3V3 para alimentar os servomotores;
Verifique os limites mínimo e máximo dos pulsos;
Comece os testes com uma faixa de movimento segura;
Se o servomotor apresentar ruídos, tremores, aquecimento ou esforço mecânico excessivo, interrompa o teste e revise a montagem.
📚 Conclusão

O projeto demonstra a evolução do controle de servomotores utilizando os periféricos do RP2040, começando com uma sequência automática de posições, passando pelo controle através de uma entrada analógica e chegando ao controle de dois atuadores com um único potenciômetro.

A progressão dos níveis permite trabalhar de forma prática com:

PWM
 │
 ├──► Controle de posição
 │
 ├──► Temporização com Pico SDK
 │
 ├──► ADC
 │
 ├──► Conversão ADC → Ângulo
 │
 └──► Controle de múltiplos servomotores


O foco principal é compreender a relação entre PWM, largura de pulso e posição angular, além de aplicar aquisição analógica e controle de múltiplos atuadores utilizando os recursos disponíveis no RP2040.
