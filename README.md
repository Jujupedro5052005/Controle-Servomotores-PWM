Desenvolver uma aplicação embarcada utilizando o Raspberry Pi Pico, o Pico SDK e a linguagem C para controlar servomotores por meio das saídas de PWM do RP2040. O desafio é dividido em três níveis progressivos: cada nível reaproveita os conceitos do anterior e acrescenta uma nova funcionalidade.

Objetivo

Aplicar a geração de PWM do RP2040 no controle de posição de servomotores, evoluindo de uma sequência automática de posições para o controle por entrada analógica e, por fim, para o controle de dois servomotores a partir de um único potenciômetro.

Referência de PWM

Para os servomotores utilizados na aula, considere como referência um período de aproximadamente 20 ms (50 Hz). A posição é determinada pela largura do pulso de controle:

aproximadamente 1,0 ms para 0 graus;
aproximadamente 1,5 ms para 90 graus;
aproximadamente 2,0 ms para 180 graus.
Esses valores são referências de laboratório. Caso o servomotor atinja o limite mecânico antes dos extremos, reduza a faixa de pulsos e registre no código os limites adotados.

Níveis do Desafio

Nível 1 - Sequência automática de posições

Controlar um servomotor utilizando uma saída de PWM do RP2040.

O software deve posicionar o servomotor, em sequência, nas seguintes posições:

0 graus;
45 graus;
90 graus;
135 graus;
180 graus;
135 graus;
90 graus;
45 graus;
0 graus.
Aguardar aproximadamente 2 segundos entre cada mudança de posição.
A temporização deve ser implementada com um recurso de timer do Pico SDK, realizando a atualização do duty cycle / nível ativo do PWM sem utilizar sleep_ms() como mecanismo principal de controle.
Nível 2 - Controle de um servomotor por potenciômetro

A partir da implementação do Nível 1, substituir a sequência automática por um controle contínuo da posição do servomotor através de uma entrada analógica.

Utilizar um potenciômetro como entrada analógica. Para padronização da bancada, utilizar GP26 / ADC0.
Ler continuamente o valor do ADC.
Mapear a faixa de leitura do potenciômetro para uma posição entre 0 e 180 graus.
Atualizar o PWM para que a posição do servomotor acompanhe a variação do potenciômetro durante a execução.
A leitura pode ser realizada com adc_read(); não é obrigatório utilizar interrupção do ADC.
Nível 3 - Controle de dois servomotores com um potenciômetro

Evoluir a solução do Nível 2 para controlar dois servomotores utilizando duas saídas PWM e a mesma entrada analógica do potenciômetro.

Na primeira metade da faixa do potenciômetro, controlar o primeiro servomotor de 0 a 180 graus.
Na segunda metade da faixa do potenciômetro, controlar o segundo servomotor de 0 a 180 graus.
Utilizar duas saídas PWM independentes, permitindo definir a posição de cada servomotor separadamente.
Os slides não especificam o comportamento de cada servomotor fora de sua metade ativa. O grupo deve definir uma convenção coerente para esse caso e documentá-la no código (por exemplo, manter a última posição ou manter o servo em um dos extremos).
Requisitos Técnicos Gerais

A implementação dos três níveis deve respeitar os seguintes requisitos:

Desenvolver o projeto em C utilizando o Pico SDK.
Gerar o sinal dos servomotores utilizando os periféricos de PWM do RP2040, sem bibliotecas externas de controle de servo.
Configurar o PWM para aproximadamente 50 Hz.
Concentrar a conversão entre ângulo e largura de pulso / valor de comparação do PWM em uma função ou bloco de código bem definido.
Identificar claramente no código os GPIOs utilizados para PWM e para o ADC.
Limitar os valores calculados à faixa de pulsos definida para o servomotor, evitando comandos fora dos limites adotados.
Manter a solução estável mesmo diante de pequenas oscilações na leitura analógica.
Alimentação e Montagem

Com apenas um servomotor (Níveis 1 e 2), não é necessário utilizar fonte externa de bancada: o servo pode ser alimentado pelo VBUS do Raspberry Pi Pico durante a atividade de laboratório.
Com dois servomotores (Nível 3), é recomendada a utilização de uma fonte externa de bancada para alimentar os servos.
Ao utilizar fonte externa, conectar em comum o GND da fonte, o GND dos servomotores e o GND do Raspberry Pi Pico.
Não alimentar servomotores pelo pino 3V3 do Pico.
Recomendação: ao usar fonte externa, alimentar os servomotores diretamente pela fonte e evitar colocar a saída positiva da fonte em paralelo com o VBUS/USB do Pico.
Caso ocorram tremores, resets ou comportamento instável, verificar primeiro a alimentação, o GND comum e os limites de pulso utilizados.
Critérios de Avaliação

Serão avaliados, de forma conjunta, os seguintes itens:

Configuração correta da frequência e dos parâmetros de PWM para o controle dos servomotores.
Conversão correta entre ângulo e largura de pulso / duty cycle.
Nível 1: execução correta da sequência de posições e respeito ao intervalo de aproximadamente 2 segundos.
Nível 1: uso adequado do mecanismo de temporização para realizar as mudanças de posição.
Nível 2: inicialização e leitura correta do ADC, com mapeamento coerente da entrada analógica para 0 a 180 graus.
Nível 2: resposta contínua e estável do servomotor ao movimento do potenciômetro.
Nível 3: configuração correta de duas saídas PWM e controle independente dos dois servomotores.
Nível 3: divisão correta da faixa do potenciômetro entre os dois servomotores e coerência da convenção adotada fora da região ativa de cada um.
Montagem elétrica adequada, incluindo a alimentação dos servomotores e o GND comum quando houver fonte externa.
Organização, clareza, legibilidade e robustez geral do código.
Observações e Sugestões

Criem uma função para converter ângulo em largura de pulso ou valor de comparação do PWM, evitando números mágicos espalhados pelo programa.
No Nível 1, uma boa estratégia é armazenar a sequência de ângulos em um vetor e utilizar um índice para avançar entre as posições.
Nos Níveis 2 e 3, se pequenas oscilações do ADC causarem tremores, pode ser utilizada uma pequena zona morta (deadband) ou uma média simples de leituras.
No Nível 3, documentem claramente o valor adotado como ponto médio do ADC e a forma de mapeamento de cada metade da faixa.
Definam constantes para frequência do PWM, largura de pulso mínima/máxima, GPIOs e limites do ADC. Isso facilita a calibração e a leitura do código.
Recomenda-se manter versões identificadas de cada nível para facilitar testes, demonstração e correção, mesmo que o código seja desenvolvido de forma incremental.
Complementos Opcionais

Os itens abaixo são opcionais e não substituem os requisitos principais:

Aplicar média móvel ou outro filtro simples à leitura do ADC.
Implementar uma pequena deadband para reduzir atualizações causadas apenas por ruído do potenciômetro.
Criar constantes de calibração para as larguras de pulso mínima, central e máxima do servomotor.
Enviar pela interface serial o valor do ADC, o ângulo calculado e/ou a largura de pulso aplicada durante a depuração.
Quando houver osciloscópio ou analisador lógico disponível, verificar experimentalmente o período de aproximadamente 20 ms e as larguras de pulso utilizadas.
Entrega

Os alunos devem:

Inserir, em comentário no topo do main.c (ou equivalente), o nome completo e o RA de todos os integrantes do grupo.
Entregar as implementações dos três níveis de forma claramente identificada. Elas podem ser organizadas como versões ou subpastas do mesmo projeto.
Entregar código funcional e compilável, incluindo os arquivos necessários para compilação com o Pico SDK / CMake.
Compactar a pasta de entrega em um único arquivo .zip.
Observações Finais

O foco principal do desafio é compreender a geração de PWM para servomotores e a conversão de diferentes referências de entrada em posição angular.
A progressão entre os níveis deve demonstrar domínio crescente: temporização, aquisição analógica e controle simultâneo de múltiplos atuadores.
Melhorias opcionais são bem-vindas, mas não compensam requisitos principais incompletos.
Se houver comportamento mecânico anormal, interrompa o acionamento e revise a montagem e os limites de pulso antes de continuar os testes.
