%UNTITLED Summary of this function goes here
%   Detailed explanation goes here

% Reading the files

function [Y] = mySobelEdge(X, threshold) 

    % X(i,j,3)
    X = X - 1;
    s = size(X);
    length = s(1);
    width = s(2);
    Y = zeros(length,width,3);
    for ch = 1:3;
        for i = 2:length-1
            for j = 2:width-1
                sum1 = X(i-1,j+1,ch) + 2*X(i,j+1,ch) + X(i+1,j-1,ch);
                sum1 = sum1 - X(i-1,j-1,ch) - 2*X(i,j-1,ch) - X(i+1,j-1,ch);
                sum2 = X(i+1,j-1,ch) + 2*X(i+1,j,ch) + X(i+1,j+1,ch);
                sum2 = sum2 - X(i-1,j-1,ch) - 2*X(i-1, j,ch) - X(i-1,j+1,ch);
                gradient = sum1.^2 + sum2.^2;
                if gradient > threshold.^2
                    Y(i,j,ch) = 255;
                else
                    Y(i,j,ch) = 0;
                end
            end
        end
    end
end
