// Autor: Eliezer Emanuel Bernart
// Data: 23/08/2014 

#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <vector>

// Variáveis de controle
int escala = 15,
    qualidade = 75;

// Matrizes para as imagens
cv::Mat imagem_entrada,
        imagem_comprimida;

void processarImagem(int, void*)
{
    // Configuração dos parametros e compressão JPEG
    std::vector<int> parametros;
    parametros.push_back(CV_IMWRITE_JPEG_QUALITY);
    parametros.push_back(qualidade);
    cv::imwrite("temp.jpg", imagem_entrada, parametros);

    // Leitura da imagem temporária do disco
    imagem_comprimida = cv::imread("temp.jpg");

    if (imagem_comprimida.empty())
    {
        std::cout << "> Erro ao carregar a imagem temporaria" << std::endl;
        exit(EXIT_FAILURE);
    }

    cv::Mat imagem_saida = cv::Mat::zeros(imagem_entrada.size(), CV_8UC3);

    // Percorre as matrizes comparando os valores
    for (int linha = 0; linha < imagem_entrada.rows; ++linha)
    {
          const uchar*  ptr_entrada = imagem_entrada.ptr<uchar>(linha);
          const uchar*  ptr_comprimida = imagem_comprimida.ptr<uchar>(linha);
                uchar*  ptr_saida = imagem_saida.ptr<uchar>(linha);

        for (int coluna = 0; coluna < imagem_entrada.cols; coluna++)
        {
            // Cálculo da diferença absoluta em cada canal de cor, multiplicado pela escala
            ptr_saida[0] = abs(ptr_entrada[0] - ptr_comprimida[0]) * escala;
            ptr_saida[1] = abs(ptr_entrada[1] - ptr_comprimida[1]) * escala;
            ptr_saida[2] = abs(ptr_entrada[2] - ptr_comprimida[2]) * escala;

            ptr_entrada    += 3;
            ptr_comprimida += 3;
            ptr_saida      += 3;
        }
    }

    // Exibe imagem processada
    cv::imshow("Error Level Analysis", imagem_saida);

}

int main (int argc, char* argv[])
{
    // Verifica se o número de parâmetros necessário foi informado
    if (argc < 2)
    {
        std::cout << "> E preciso informar uma imagem como parametro" << std::endl;
        return EXIT_FAILURE;
    }

    // Lê a imagem passada por parâmetro
    imagem_entrada = cv::imread(argv[1]);

    // Verifica se a imagem foi carregada corretamente
    if (imagem_entrada.empty())
    {
        std::cout << "> Erro ao carregar a imagem de entrada" << std::endl;
        return EXIT_FAILURE;
    }

    // Configuração da janela e dos componentes
    cv::namedWindow("Error Level Analysis", CV_WINDOW_AUTOSIZE);
    cv::imshow("Error Level Analysis", imagem_entrada);
    cv::createTrackbar("Escala", "Error Level Analysis", &escala, 100, processarImagem);
    cv::createTrackbar("Qualidade", "Error Level Analysis", &qualidade, 100, processarImagem);

    // Aguarda até que a tela 'q' seja pressionada para encerrar o programa
    while (char(cv::waitKey(0)) != 'q') {};

    return EXIT_SUCCESS;
}
