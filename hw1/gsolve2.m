%

function [g] = gsolve2(Z,M)

    N = size(Z,1);
    P = size(Z,2);
    C = zeros(M+1,1); %C(1:M+1)
    C(M) = 1;
    err = Inf(1);
    
    for xxx = 1:10
        R = zeros(P-1,1);
        for j = 1:P-1
            for i = 1:N
                a = polyval(C, Z(i,j));
                b = polyval(C, Z(i,j+1));
                R(j) = R(j) + a/b;
            end
            R(j) = R(j)/N;
        end
         
        d = zeros(N,P-1,M+1);
        for i = 1:N
            for j = 1:P-1
                for m = 1:M+1
                    d(i,j,m) = Z(i,j).^(m-1) - R(j)*(Z(i,j+1).^(m-1));
                end
            end
        end
         
        A = zeros(M,M);
        for x = 1:M
            for y = 1:M
                for i = 1:N
                    for j = 1:P-1
                        A(x,y) = A(x,y) + d(i,j,x)*(d(i,j,y)-d(i,j,M+1));
                    end
                end
            end
        end
         
        B = zeros(M,1);
        for m = 1:M
            for i = 1:N
                for j = 1:P-1
                    B(m) = B(m) - d(i,j,m)*d(i,j,M+1);
                end
            end
        end
 
        C2 = zeros(M+1,1); %C2(1:M+1)
        C2(1:M) = A\B; %AC = B
        C2(M+1) = 1 - sum(C2(1:M));
        err2 = 0;
        for i = 1:N
            for j = 1:P-1
                a = polyval(C, Z(i,j));
                b = R(j)*polyval(C, Z(i,j+1));
                err2 = err2 + (a-b).^2;
            end
        end
        if err2 < err
            err = err2;
            C = C2;
        end
    end

    g = zeros(256,1);
    gmax = polyval(C,1/256);
    for x = 1:256
        g(x) = polyval(C,x/256)/gmax;
    end
    g = flipud(g);
    figure;
    plot(linspace(0,1,256),g);

end
