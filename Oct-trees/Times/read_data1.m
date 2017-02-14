function [results] = read_data(nL, nP, nN, iter,threads)
% function [results] = read_data(src, nL, nP, nN,iter)
% 
% input: 
%   src: The file containing the raw results, produced by the run_tests.sh script 
%   nL: The number of different tree heights tested
%   nP: The number of different population thresholds tested
%   nN: The number of different number of particles tested
%   iter: Number of repetitions of a single data point
%
% output:
%   results: The structure with the final results 
%       .hash: The time to compute the hash codes
%       .morton: The time to compute the morton encoding
%       .sort: The time of the truncated sort
%       .dataR: The time for data rearrangement
%
% author: Nikos Sismanis
% date: 22 Nov 2014

%system(sprintf('findstr /C:"Time to compute the hash codes" %s > hash.txt','r', src));
%system(sprintf('findstr /C:"Time to compute the morton encoding" %s > morton.txt','r', src));
%system(sprintf('findstr /C:"Time for the truncated radix sort" %s > sort.txt','r', src));
%system(sprintf('findstr /C:"Time to rearrange the particles in memory" %s > data_rear.txt','r', src));


fid1 = fopen('pthread_hash_plummer.txt','r');
hash_time1 = fscanf(fid1, 'Time to compute the hash codes            : %fs\n');
fclose(fid1);

hash_mean1 = mean(reshape(hash_time1, iter, length(hash_time1) / iter), 1);

hash_mean1 = reshape(hash_mean1,threads,nL*nP*nN);



fid5 = fopen('hash_plummer.txt','r');
hash_time2 = fscanf(fid5, 'Time to compute the hash codes            : %fs\n');
fclose(fid5);

hash_mean2 = mean(reshape(hash_time2, iter, length(hash_time2) / iter), 1);
%hash_mean2 = reshape(hash_mean2,nL*nP*nN,1,1);

fid2 = fopen('pthread_morton_plummer.txt','r');
morton_time1 = fscanf(fid2, 'Time to compute the morton encoding       : %fs\n');
fclose(fid2);

morton_mean1 = mean(reshape(morton_time1, iter, length(morton_time1) / iter), 1);
morton_mean1 = reshape(morton_mean1,threads,nL*nP*nN);
%
fid6 = fopen('morton_plummer.txt','r');
morton_time2 = fscanf(fid6, 'Time to compute the morton encoding       : %fs\n');
fclose(fid6);

morton_mean2 = mean(reshape(morton_time2, iter, length(morton_time2) / iter), 1);
%morton_mean2 = reshape(morton_mean2,nL*nP*nN,1,1);

fid3 = fopen('pthread_radix_plummer.txt','r');
sort_time1 = fscanf(fid3, 'Time for the truncated radix sort         : %fs\n');
fclose(fid3);

sort_mean1 = mean(reshape(sort_time1, iter, length(sort_time1) / iter), 1);
sort_mean1 = reshape(sort_mean1,threads,nL*nP*nN);

fid7 = fopen('radix_plummer.txt','r')
sort_time2 = fscanf(fid7, 'Time for the truncated radix sort         : %fs\n');
fclose(fid7);

sort_mean2 = mean(reshape(sort_time2, iter, length(sort_time2) / iter), 1);
%sort_mean2 = reshape(sort_mean2,nL*nP*nN,1,1);

fid4 = fopen('pthread_data_plummer.txt','r');
dataR_time1 = fscanf(fid4, 'Time to rearrange the particles in memory : %fs\n');
fclose(fid4);

dataR_mean1 = mean(reshape(dataR_time1, iter, length(dataR_time1) / iter), 1);
dataR_mean1 = reshape(dataR_mean1,threads,nL*nP*nN);

fid8 = fopen('data_plummer.txt','r');
dataR_time2 = fscanf(fid8, 'Time to rearrange the particles in memory : %fs\n');
fclose(fid8);

dataR_mean2 = mean(reshape(dataR_time2, iter, length(dataR_time2) / iter), 1);
%dataR_mean2 = reshape(dataR_mean2,nL*nP*nN,1,1);

results.hash1 = hash_mean1;
results.morton1 = morton_mean1;
results.sort1 = sort_mean1;
results.dataR1 = dataR_mean1;

results.hash2 = hash_mean2;
results.morton2 = morton_mean2;
results.sort2 = sort_mean2;
results.dataR2 = dataR_mean2;

disp(dataR_mean1);
disp(dataR_mean2);
sum1=hash_mean1+morton_mean1+sort_mean1+dataR_mean1;
sum2=hash_mean2+morton_mean2+sort_mean2+dataR_mean2;


%c = hash_mean2 ./ hash_mean1
C = bsxfun(@rdivide, hash_mean2, hash_mean1);
%disp(C);

%a=results.hash1./results.hash2;
%b=results.morton1./results.morton2;
%c=results.sort1./results.sort2;
%d=results.dataR1./results.dataR2;

x = [0,1,2,3,4,5,6,7,8,9,10,11];

C = bsxfun(@rdivide, hash_mean2, hash_mean1);
subplot(3,2,1);
plot(x,C);
legend('2^{20}','2^{21}','2^{22}','2^{23}','2^{24}','2^{25}');
xlabel('2^{Number of Threads}');
ylabel('Acceleration');
title('Hash Codes');

C = bsxfun(@rdivide, morton_mean2, morton_mean1);
subplot(3,2,2);
plot(x,C);
legend('2^{20}','2^{21}','2^{22}','2^{23}','2^{24}','2^{25}');
xlabel('2^{Number of Threads}');
ylabel('Acceleration');
title('Morton Encoding');

C = bsxfun(@rdivide, sort_mean2, sort_mean1);
subplot(3,2,3);
plot(x,C);
legend('2^{20}','2^{21}','2^{22}','2^{23}','2^{24}','2^{25}');
xlabel('2^{Number of Threads}');
ylabel('Acceleration');
title('Radix Sort');

C = bsxfun(@rdivide, dataR_mean2, dataR_mean1);
disp(C);
subplot(3,2,4);
plot(x,C);
legend('2^{20}','2^{21}','2^{22}','2^{23}','2^{24}','2^{25}');
xlabel('2^{Number of Threads}');
ylabel('Acceleration');
title('Data rearrangement');

C = bsxfun(@rdivide, sum2, sum1);
subplot(3,2,5);
plot(x,C);
legend('2^{20}','2^{21}','2^{22}','2^{23}','2^{24}','2^{25}');
xlabel('2^{Number of Threads}');
ylabel('Acceleration');
title('Total');

end