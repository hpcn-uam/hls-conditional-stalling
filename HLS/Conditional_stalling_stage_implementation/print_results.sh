cd logs
for f in *_impl.log;do
  period=`awk -F " " '/post-implementation/{print $4}' $f`;
  LUT=`awk -F " " '/LUT/{print $2}' $f|tail -1`;
  FF=`awk -F " " '/FF/{print $2}' $f|tail -1`;
  BRAM=`awk -F " " '/BRAM/{print $2}' $f|tail -1`;
  URAM=`awk -F " " '/URAM/{print $2}' $f|tail -1`;
  SLICE=`awk -F " " '/SLICE/{print $2}' $f|tail -1`;
  CLB=`awk -F " " '/CLB/{print $2}' $f|tail -1`;

  echo -n "$f| period: $period";
  echo -n "| LUT: $LUT";
  echo -n "| FF: $FF";
  echo -n "| BRAM: $BRAM";
  echo -n "| URAM: $URAM";
  echo -n "| SLICE: $SLICE";
  echo  "| CLB: $CLB";
done
