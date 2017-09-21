%UNTITLED Summary of this function goes here
%   Detailed explanation goes here

function [rgb] = myTonemap(hdr,algorithm,key,arg1,arg2)
    % hdr(i,j,3)
    length = size(hdr,1);
    width = size(hdr,2);
    N = length*width;
    L = zeros(length,width);
    delta = realmin('single');
    rgb = zeros(length,width,3);
    wr = 0.299;
    wb = 0.114;
    wg = 1 - wr - wb;

    Y = wr*hdr(:,:,1) + wg*hdr(:,:,2) + wb*hdr(:,:,3);

    for i = 1:length
        for j = 1:width
            if Y(i,j) == 0
                Y(i,j) = delta;
            end
        end
    end

    Lw = geomean(reshape(Y,N,1));

    for i = 1:length
        for j = 1:width
            L(i,j) = key*Y(i,j)/Lw;
        end
    end

    Ld = zeros(length,width);
    if strcmp(algorithm,'global')
        if exist('arg1','var')
            Lwhite = arg1;
        else
            Lwhite = max(max(L));
        end
        Lwh2 = Lwhite.^2;
        Ld = L.*(1+L/Lwh2)./(1+L);
    elseif strcmp(algorithm,'local')
        Smax = 8;
        S = [1 7 13 19 25 31 37 43];
        Ls = zeros(Smax,length,width);
        for s = 1:Smax
            Ls(s,:,:) = imgaussfilt(L,S(s));
        end
        Lwhite = max(max(L));
        phi = arg1;
        err = arg2;
        b0 = (2.^phi)*0.35;
        for i = 1:length
            for j = 1:width
                Ld(i,j) = Lw;
                for s = 1:Smax
                    a = Ls(1,i,j) - Ls(s,i,j);
                    b = b0/(S(s).^2) + Ls(1,i,j);
                    Vs = a/b;
                    if abs(Vs) >= err || s == Smax
                        Ld(i,j) = L(i,j)/(1+Ls(s,i,j));
                        break;
                    end
                end
            end
        end
    end
    for i = 1:length
        for j = 1:width
            rgb(i,j,:) = Ld(i,j)*(hdr(i,j,:)/Y(i,j)).^0.6;         
        end
    end
    figure;
    imshow(rgb);
    axis image;
end
