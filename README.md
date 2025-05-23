# ⛈️ DogStation - BitDogLab ⛈️

A DogStation tem como objetivo simular o funcionamento de uma estação de monitoramento de cheias através da BitDogLab onde, por meio da leitura dos analógicos da placa, serão simuladas as leituras de sensores que verificam nível de água e volume de chuva. O projeto visa aprofundar o conhecimento em FreeRTOS, dessa vez com a utilização de queues (filas) para o envio de forma eficiente das informações entre as tasks desenvolvidas.

Desenvolvido por: Taylan Mayckon

---
## 📽️ Link do Video de Demonstração:
[YouTube](https://youtu.be/A3UUMXyyoU0)

---

## 📌 **Funcionalidades Implementadas**

- FreeRTOS para geração de diferentes Tasks: Foram geradas cinco tasks para o desenvolvimento do projeto
- Simulação das leituras de nível de água e volume de chuva através da leitura dos Joysticks da BitDogLab
- Interpretação dessas leituras em possíveis sinais de alerta: Quando o nível de água passa de 70% ou o volume de chuva passa de 80% são geradas flags para indicar os alertas, sendo os individuais e o crítico quando ocorre ambos
- Envio de dados para as demais tasks do código por meio de queues
- Animações interativas na Matriz de LEDs: Quando não está sobre alerta, o sistema exibe os níveis de água e volume de chuva de forma dinâmica na matriz. Em modo de alerta é exibida uma animação de exclamação pulsando em amarelo.
- Emissão de sinais de socorro: Quando em modo de alerta, o Led RGB representa o envio de uma mensagem de socorro em código morse, um SOS
- Alertas sonoros para deficientes visuais: Os buzzers são acionados quando o sistema entra em modo de alerta, dessa forma são emitidos três diferentes padrões de alerta
- Mensagens informativas no Display OLED: No display OLED é possível ver as informações de nível da água e volume de chuva, tanto no formato numérico quanto também com uma barra dinâmica que se preenche conforme a porcentagem de cada um. Em modo de alerta são emitidas mensagens no display.

---

## 🛠 **Recursos Utilizados**

- **FreeRTOS:** é um sistema operacional de código aberto e tempo real (RTOS) projetado para microcontroladores e dispositivos embarcados. Ele permite a criação de diferentes tarefas e faz o gerenciamento das mesmas para serem executadas de forma paralela.
- **Queues:** é um recurso utilizado com FreeRTOS, ele auxilia a enviar informações entre as tasks de forma eficiente.
- **Display OLED:** foi utilizado o ssd1306, que possui 128x64 pixels, para informações visuais os dados lidos.
- **Matriz de LEDs Endereçáveis:** A BitDogLab possui uma matriz de 25 LEDs WS2812, que foi operada com o auxílio de uma máquina de estados.
- **LED RGB:** Utilizado para emitir sinais de SOS em código morse quando em alertas.
- **Buzzers:** Emite sons para gerar alertas sonoros para deficientes auditivos.

---

## 📸 **Imagens do Funcionamento**

São enviados Logs periódicos para monitorar os dados que estão sendo lidos e enviados para as outras tasks.
<p align="center">
  <img src="https://github.com/taylanmayckon/AlertaEnchentes/blob/99cb1a16186f1ed6a12fb30923b5c73be1225c5d/images/EnvioDeLogs.png" alt="Envio de Logs" width="450"><br>
  <strong>Envio de Logs</strong>
</p>

As porcentagens atuais podem ser visualizadas através da Matriz de LEDs.
<p align="center">
  <img src="https://github.com/taylanmayckon/AlertaEnchentes/blob/99cb1a16186f1ed6a12fb30923b5c73be1225c5d/images/Funcionamento1.jpg" alt="Niveis na Matriz" width="450"><br>
  <strong>Níveis exibidos na Matriz de LEDs WS2812</strong>
</p>

Dados lidos e mensagens são visualizados através do Display OLED.
<p align="center">
  <img src="https://github.com/taylanmayckon/AlertaEnchentes/blob/99cb1a16186f1ed6a12fb30923b5c73be1225c5d/images/Funcionamento2.jpg" alt="Display OLED" width="450"><br>
  <img src="https://github.com/taylanmayckon/AlertaEnchentes/blob/99cb1a16186f1ed6a12fb30923b5c73be1225c5d/images/Funcionamento3.jpg" alt="Display OLED" width="450"><br>
  <img src="https://github.com/taylanmayckon/AlertaEnchentes/blob/99cb1a16186f1ed6a12fb30923b5c73be1225c5d/images/Funcionamento4.jpg" alt="Display OLED" width="450"><br>
  <strong>Mensagens Emitidas no Display SSD1306</strong>
</p>

---

## 📂 **Estrutura do Código**

```
📂 AlertaEnchentes/
├── 📄 AlertaEnchentes.c               # Código principal do projeto
├──── 📂lib
├───── 📄 FreeRTOSConfig.h             # Arquivos de configuração para o FreeRTOS
├───── 📄 font.h                       # Fonte utilizada no Display I2C
├───── 📄 led_matrix.c                 # Funções para manipulação da matriz de LEDs endereçáveis
├───── 📄 led_matrix.h                 # Cabeçalho para o led_matrix.c
├───── 📄 ssd1306.c                    # Funções que controlam o Display I2C
├───── 📄 ssd1306.h                    # Cabeçalho para o ssd1306.c
├───── 📄 structs.h                    # Structs utilizadas no código principal
├───── 📄 ws2812.pio                   # Máquina de estados para operar a matriz de LEDs endereçáveis
├── 📄 CMakeLists.txt                  # Configurações para compilar o código corretamente
└── 📄 README.md                       # Documentação do projeto
```

---
