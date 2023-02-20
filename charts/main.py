import plotly.express as px
import re
import plotly as py

def create_graphs():
    file = open("../build/OldOutput/Output/results.csv")
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
        algo = line[0:line.find("_")]

#        if algo == "Hilbert" and not ((quantization == 10 and parameter == 2) or (quantization == 12 and parameter == 2) or (quantization == 14)
#            or (quantization == 16 and parameter == 4)):
#            continue

        csv = line[line.index(",") + 1: len(line)].split(",")
        maxErr = float(csv[0])
        avgErr = float(csv[1])
        maxDespeckledErr = float(csv[2]) if csv[2] != "/" else 0
        avgDespeckledErr = float(csv[3]) if csv[3] != "/" else 0
        jpegSize = float(csv[4]) / 1000

        algoNames.append(algo)
        errors.append(avgErr)
        jpegSizes.append(jpegSize)
        labels.append(config)
        quantizations.append(quantization)
        jpegQualities.append(jpeg)
        params.append(parameter)
        uneditedJpeg.append(numbers[1])


    data = list(zip(errors, jpegSizes, labels, quantizations, jpegQualities, params, uneditedJpeg, algoNames))
    data.sort(key=lambda x: x[1])

    errors, jpegSizes, configs, quantizations, jpegQualities, params, uneditedJpeg, algoNames = (zip(*data))
    errors = list(errors)
    jpegSizes = list(jpegSizes)
    configs = list(configs)
    quantizations = list(quantizations)
    jpegQualities = list(jpegQualities)
    params = list(params)
    uneditedJpeg = list(uneditedJpeg)
    algoNames = list(algoNames)

    fig = px.scatter(x=jpegSizes, y=errors, labels={"x": "Compressed texture size", "y": "Logarithmic mean error"}, color=quantizations, size=jpegQualities,
                     hover_data={"Parameter": params, "Jpeg quality": uneditedJpeg}, symbol=algoNames, title="PNG")
    fig.show()
    fig.update_layout({  "xaxis": {    "type": "category"  }})


# Press the green button in the gutter to run the script.
if __name__ == '__main__':
    create_graphs()

# See PyCharm help at https://www.jetbrains.com/help/pycharm/
