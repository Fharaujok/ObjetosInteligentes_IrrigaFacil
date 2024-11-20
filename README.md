# IrrigaFácil
### Projeto da Matéria de Objetos Inteligentes Conectados - IrrigaFácil

#### Autor: Fernando Henrique Souza de Araújo

Este projeto, denominado IrrigaFácil, tem como objetivo medir a umidade do solo e irrigar plantas de maneira automatizada. Utiliza um módulo NodeMCU ESP8266 juntamente com um sensor de umidade do solo (higrômetro), acionando quando necessário uma bomba de água através de um relé. O sistema é controlado via protocolo MQTT, permitindo acesso a uma dashboard para verificar o valor atual da umidade do solo e irrigar se necessário.

### Hardware Utilizado
- Módulo Wi-Fi NodeMCU ESP8266
- Sensor de Umidade do Solo YL-69
- Protoboard 830 Pontos
- Kit de Jumpers
- Módulo Relé 4 Canais 3V 10A com Borne KRE para ESP32
- Mini Bomba de Água Motor 12V DC - RS-385
- Mangueira Cristal
- Cabos USB
- Fonte 9V 1A Bivolt para Arduino

### Comunicação MQTT
Na parte de comunicação com o MQTT, utilizei o broker test.mosquitto.org apontando para a porta 1883. Optei por um broker público por conta da simplicidade na execução dos testes. Na implementação do sistema, criamos os seguintes tópicos:

- sensor/umidade: Neste tópico, o ESP8266 publica o valor atual da umidade do solo em formato JSON, incluindo um timestamp para referência temporal.
- irrigador/controle: Tópico para controle do sistema, onde é possível enviar comandos como "ligar" ou "desligar" para controlar manualmente a bomba de água.
- irrigador/status: Tópico onde o ESP8266 publica mensagens de status após receber comandos, indicando ações como "Relé acionado manualmente" ou "Relé desligado manualmente".

Referências
- Repositório do Projeto: GitHub - IrrigaFácil
- Vídeo de Demonstração: [YouTube - IrrigaFácil em Ação](https://youtu.be/_imMiZyLVVk)
- Broker MQTT Público: Mosquitto Test Server
