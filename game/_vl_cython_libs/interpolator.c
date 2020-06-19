/*
@author: Vladya
*/


double _lagrangePolynomialsInterpolation(
    const double x,            // Argument (in Hz).
    const float *_bufferData,  // FFT data.
    const int bufferLen,
    const double stepSize      // Step size between array elements (in Hz).
) {
    /*
    Interpolates the function using the Lagrange method
    according to the data in the buffer
    and returns the value of the resulting function.
    -1. if error.
    */

    double y = .0;

    int i, j;
    double xi, xj, yi, multipler;
    for (i = 0; (i < bufferLen); i++) {
        xi = (stepSize * i);
        yi = _bufferData[i];
        multipler = 1.;
        for (j = 0; (j < bufferLen); j++) {
            if (j != i) {
                xj = (stepSize * j);
                multipler *= ((x - xj) / (xi - xj));
            };
        };
        y += (yi * multipler);
    };
    return y;
};


double _linearInterpolation(
    const double x,
    const float *_bufferData,
    const int bufferLen,
    const double stepSize
) {

    if (x <= .0) {
        return _bufferData[0];
    } else if (x >= (stepSize * (bufferLen - 1))) {
        return _bufferData[(bufferLen - 1)];
    };

    double x1, y1, x2, y2, coefficient;
    x1 = .0;
    y1 = _bufferData[0];

    int i;
    for (i = 1; (i < bufferLen); i++) {
        x2 = (stepSize * i);
        y2 = _bufferData[i];
        if ((x1 <= x) && (x < x2)) {
            coefficient = ((x - x1) / (x2 - x1));
            return (y1 + ((y2 - y1) * coefficient));
        };
        x1 = x2;
        y1 = y2;
    };
    return (-1.);
};
