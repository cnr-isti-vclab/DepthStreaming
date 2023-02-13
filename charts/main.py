import plotly.express as px
import re
import plotly as py

def create_graphs():
    file = open("../build/Output/results.csv")
    lines = file.readlines()

    labels = []
    jpegSizes = []
    algoNames = []
    errors = []
    jpegQualities = []
    quantizations = []
    params = []
    uneditedJpeg = []

    for line in lines:
        numbers = re.findall(r'\d+', line)
        if len(numbers) == 0:
            continue
        config = line.split(",")[0]
        quantization = int(numbers[0])
        jpeg = int(((float(numbers[1]) - 60) / 30) * 100)
        parameter = int(numbers[2])

        csv = line[line.index(",") + 1: len(line)].split(",")
        maxErr = float(csv[0])
        avgErr = float(csv[1])
        maxDespeckledErr = float(csv[2])
        avgDespeckledErr = float(csv[3])
        jpegSize = float(csv[4]) / 1000

        algoNames.append(line[0:line.find("_")])
        errors.append(avgDespeckledErr)
        jpegSizes.append(jpegSize)
        labels.append(config)
        quantizations.append(quantization)
        jpegQualities.append(jpeg)
        params.append(parameter)
        uneditedJpeg.append(numbers[1])


    data = list(zip(errors, jpegSizes, labels, quantizations, jpegQualities, params, uneditedJpeg, algoNames))
    data = sorted(data, key=lambda x: x[1])

    errors, jpegSizes, configs, quantizations, jpegQualities, params, uneditedJpeg, algoNames = (zip(*data))
    errors = list(errors)
    jpegSizes = list(jpegSizes)
    configs = list(configs)
    quantizations = list(quantizations)
    jpegQualities = list(jpegQualities)
    params = list(params)
    uneditedJpeg = list(uneditedJpeg)
    algoNames = list(algoNames)

    for i in range(0, len(jpegSizes)):
        jpegSizes[i] = str(jpegSizes[i]) + "KB"

    fig = px.scatter(x=jpegSizes, y=errors, labels={"x": "Compressed texture size", "y": "Logarithmic mean error"}, color=quantizations, size=jpegQualities,
                     hover_data={"Parameter": params, "Jpeg quality": uneditedJpeg}, symbol=algoNames)
    fig.show()


# Press the green button in the gutter to run the script.
if __name__ == '__main__':
    create_graphs()

# See PyCharm help at https://www.jetbrains.com/help/pycharm/
