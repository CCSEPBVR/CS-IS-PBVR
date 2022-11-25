# -*- coding: utf-8 -*-



def read_coordinates
  begin
    f = File.open("co3d.dat", "rb")
    for z in 1..48 do
      for y in 1..220 do
        for x in 1..220 do
          vb = f.read(12)
          val = vb.unpack("f*")
          if (z==1 && y==1) || (z==48 && y==220) then
            printf("%5d%5d%5d%10.3f%10.3f%10.3f\n", z, y, x, val[0], val[1], val[2])
          end
        end
      end
    end
  rescue => e
    print e
  ensure
    f.close
  end
end


# "psid.dat" の中身を見てみる。
def test_read_psid_dat(dim1, dim2, dim3, skip)
  # data=float
  ndivx = 4       # X方向の出力ギャップ
  ndivy = 4
  ndivz = 4
  
  begin
    f = File.open("psid.dat", "rb")
    fo = File.open("psid.txt", "w")
    
    f.read(skip)
    for z in 1..dim3 do
      for y in 1..dim2 do
        vb = f.read(4*dim1)
        valx = vb.unpack("f*")
        
        if (y%ndivy==0 && z%ndivz==0) then
          for x in 0..dim1-1 do
            if ((x+1)%ndivx==0) then
              fo.write("%8.3f" % valx[x])
            end
          end
          fo.write("\n")
        end
      end
      if (z%ndivz==0) then
        fo.write("\n\n")
      end
    end
  rescue => e
    print e
  ensure
    f.close
    fo.close
  end
end


# "pd3d.dat" の中身を見てみる。
def test_read_pd3d_dat(dim1, dim2, dim3, skip, istep=0)
  # data=float
  ndivx = 4       # X方向の出力ギャップ
  ndivy = 4
  ndivz = 4
  
  stepSize = dim1*dim2*dim3*4 + skip    # byte
  seekPos = istep*stepSize + skip
  
  begin
    f = File.open("pd3d.dat", "rb")
    
    outFilename = sprintf("pd3d-%05d.txt", istep+1)
    fo = File.open(outFilename, "w")
    
    f.seek(seekPos)
    for z in 1..dim3 do
      for y in 1..dim2 do
        vb = f.read(4*dim1)
        valx = vb.unpack("f*")
        
        if (y%ndivy==0 && z%ndivz==0) then
          for x in 0..dim1-1 do
            if ((x+1)%ndivx==0) then
              fo.write("%8.3f" % valx[x])
            end
          end
          fo.write("\n")
        end
      end
      if (z%ndivz==0) then
        fo.write("\n\n")
      end
    end
  rescue => e
    print e
  ensure
    f.close
    fo.close
  end
end



# "psid.dat" ファイルをバイナリコピーする。
def copy_psid_dat_be(fp, dim1, dim2, dim3, skip)
  # data=float
  
  begin
    f = File.open("psid.dat", "rb")
    
    f.read(skip)
    for z in 1..dim3 do
      vb = f.read(4*dim1*dim2)
      #fp.write(vb)
      fp.write(vb.unpack("N*").pack("V*"))      # little endian -> big endian
    end
  rescue => e
    print e
  ensure
    f.close
  end
end


# "pd3d.dat" の指定したステップのデータをコピーする。
def copy_pd3d_dat_be(fp, dim1, dim2, dim3, skip, istep=0)
  # data=float
  
  stepSize = dim1*dim2*dim3*4 + skip    # byte
  seekPos = istep*stepSize + skip
  
  begin
    f = File.open("pd3d.dat", "rb")
    f.seek(seekPos)
    for z in 1..dim3 do
      vb = f.read(4*dim1*dim2)
      #fp.write(vb)
      fp.write(vb.unpack("N*").pack("V*"))      # little endian -> big endian
    end
  rescue => e
    print e
  ensure
    f.close
  end
end


# "co3d.dat" の値をコピーする。(big endianに変換)
def copy_co3d_dat_be(fp, dim1, dim2, dim3, skip)
  seekPos = skip
  begin
    f = File.open("co3d.dat", "rb")
    f.seek(seekPos)
    for z in 1..dim3 do
      vb = f.read(12*dim1*dim2)                 # float*(x,y,z)
      fp.write(vb.unpack("N*").pack("V*"))      # little endian -> big endian
    end
  rescue => e
    print e
  ensure
    f.close
  end
end


# "co3d.dat" の1つの軸の値をコピーする。(big endianに変換)
#
# @param axisName 'x', 'y', or 'z'。出力する軸
# @param indexInAxis1 axisNameで指定した以外の軸の1つ目の軸のインデックス
# @param indexInAxis2 axisNameで指定した以外の軸の2つ目の軸のインデックス
#
def copy_axis_co3d_dat_be(fp, dim1, dim2, dim3, axisName, indexInAxis1, indexInAxis2)
  # no skip
  begin
    f = File.open("co3d.dat", "rb")
    if (axisName=='x') then
      for z in 1..dim3 do
        if (indexInAxis2 == z-1) then
          for y in 1..dim2 do
            if (indexInAxis1 == y-1) then
              for x in 1..dim1 do
                bx = f.read(4)
                by = f.read(4)
                bz = f.read(4)
                fp.write(bx.unpack("N*").pack("V*"))
              end
            else
              f.read(12*dim1)
            end
          end
        else
          f.read(12*dim1*dim2)
        end
      end
    
    elsif (axisName=='y') then
      for z in 1..dim3 do
        if (indexInAxis2 == z-1) then
          for y in 1..dim2 do
            for x in 1..dim1 do
              if (indexInAxis1 == x-1) then
                bx = f.read(4)
                by = f.read(4)
                bz = f.read(4)
                fp.write(by.unpack("N*").pack("V*"))
              else
                f.read(12)
              end
            end
          end
        else
          f.read(12*dim1*dim2)
        end
      end
    
    elsif (axisName=='z') then
      for z in 1..dim3 do
        for y in 1..dim2 do
          if (indexInAxis2 == y-1) then
            for x in 1..dim1 do
              if (indexInAxis1 == x-1) then
                bx = f.read(4)
                by = f.read(4)
                bz = f.read(4)
                fp.write(bz.unpack("N*").pack("V*"))
              else
                f.read(12)
              end
            end
          else
            f.read(12*dim1)
          end
        end
      end
    end
  rescue => e
    print e
  ensure
    f.close
  end
end

# 
def copy_pd3d_dat_unsigned_char(fp, dim1, dim2, dim3, skip, istep)
  stepSize = dim1*dim2*dim3*4 + skip    # byte
  seekPos = istep*stepSize + skip
  
  begin
    min = 1e10
    max = -1e10
    
    f = File.open("pd3d.dat", "rb")
    f.seek(seekPos)
    
    # min, maxを探す
    for z in 1..dim3 do
      for y in 1..dim2 do
        vb = f.read(4*dim1)
        valx = vb.unpack("f*")
        for x in 0..dim1-1 do
          min = valx[x] if valx[x]<min
          max = valx[x] if valx[x]>max
        end
      end
    end
    
    f.seek(seekPos)
    diff = max - min
    print "min=" + min.to_s + ", max=" + max.to_s + ", diff=" + diff.to_s
    
    # charに変換してpackして出力
    for z in 1..1 do
      for y in 1..1 do
        vb = f.read(4*dim1)
        valx = vb.unpack("f*")
        for x in 0..dim1-1 do
          valx[x] = ((valx[x] - min) / diff * 255.0).ceil
          valx[x] = 0  if valx[x] < 0
          valx[x] = 255 if valx[x] > 255
        end
        fp.write(valx.pack("C*"))
      end
    end
  rescue => e
    print e
  ensure
    f.close
  end
end


# "psid.dat" ファイルをバイナリコピーする。
def copy_psid_dat_unsigned_char(fp, dim1, dim2, dim3, skip)
  # data=float
  
  seekPos = skip
  min = 1e10
  max = -1e10
  
  begin
    f = File.open("psid.dat", "rb")
    
    f.seek(seekPos)
    
    # min, maxを探す
    for z in 1..dim3 do
      for y in 1..dim2 do
        vb = f.read(4*dim1)
        valx = vb.unpack("f*")
        for x in 0..dim1-1 do
          min = valx[x] if valx[x]<min
          max = valx[x] if valx[x]>max
        end
      end
    end
    
    f.seek(seekPos)
    diff = max - min
    print "min=" + min.to_s + ", max=" + max.to_s + ", diff=" + diff.to_s, "\n"
    
    # charに変換してpackして出力
    for z in 1..dim3 do
      for y in 1..dim2 do
        vb = f.read(4*dim1)
        valx = vb.unpack("f*")
        for x in 0..dim1-1 do
          valx[x] = ((valx[x] - min) / diff * 255.0).ceil
          valx[x] = 0  if valx[x] < 0
          valx[x] = 255 if valx[x] > 255
        end
        fp.write(valx.pack("C*"))
      end
    end
  rescue => e
    print e
  ensure
    f.close
  end
end



# StructuredPoints (FIELD)のバイナリファイルを作成する。
#
def output_structured_points_vtk_bin_field(filename, dim1, dim2, dim3, spacex, spacey, spacez, istep)
  begin
    f = File.open(filename, "wb")
    f.write("# vtk DataFile Version 2.0\n")
    f.write("PBVR sample GT5D - VTK binary\n")
    f.write("BINARY\n")
    f.write("DATASET STRUCTURED_POINTS\n")
    f.write("DIMENSIONS %d %d %d\n" % [ dim1, dim2, dim3 ])
    f.write("ORIGIN %d %d %d\n" % [ 0, 0, 0 ])
    f.write("SPACING %f %f %f\n" % [ spacex, spacey, spacez])
    f.write("\n")
    f.write("POINT_DATA %d\n" % [ dim1*dim2*dim3 ])
    f.write("FIELD FieldData 2\n")
    # pd3d のデータ
    f.write("pd3d 1 %d float\n" % [ dim1*dim2*dim3 ])
    copy_pd3d_dat_be(f, dim1, dim2, dim3, 4, istep)
    # psid のデータ
    f.write("psid 1 %d float\n" % [ dim1*dim2*dim3 ])
    copy_psid_dat_be(f, dim1, dim2, dim3, 4)
  rescue => e
    print e
  ensure
    f.close
  end
end



# StructuredPoints のバイナリファイルを作成する。
# ボリュームレンダリング用
#
def output_structured_points_legacy_bin_with_scalars(filename, dim1, dim2, dim3, spacex, spacey, spacez, istep, isPd3d=true, isChar=true)
  begin
    f = File.open(filename, "wb")
    f.write("# vtk DataFile Version 2.0\n")
    f.write("PBVR sample GT5D - VTK binary\n")
    f.write("BINARY\n")
    f.write("DATASET STRUCTURED_POINTS\n")
    f.write("DIMENSIONS %d %d %d\n" % [ dim1, dim2, dim3 ])
    f.write("ORIGIN %d %d %d\n" % [ 0, 0, 0 ])
    f.write("SPACING %f %f %f\n" % [ spacex, spacey, spacez])
    f.write("\n")
    f.write("POINT_DATA %d\n" % [ dim1*dim2*dim3 ])
    if isChar then
      f.write("SCALARS scalars unsigned_char 1\n")
      f.write("LOOKUP_TABLE default\n")
    else
      f.write("SCALARS scalars float 1\n")
      f.write("LOOKUP_TABLE default\n")
    end
    if isPd3d then
      # pd3d のデータ
      printf("OUTPUT PD3D data for volume rendering\n")
      if isChar then
        copy_pd3d_dat_unsigned_char(f, dim1, dim2, dim3, 4, istep)
      else
        copy_pd3d_dat_be(f, dim1, dim2, dim3, 4, istep)
      end
    else
      # psid のデータ
      printf("OUTPUT PSID data for volume rendering\n")
      if isChar then
        copy_psid_dat_unsigned_char(f, dim1, dim2, dim3, 4)
      else
        #n = f.pos%4
        #f.write("\n") if n==3
        #f.write("\n\n") if n==2
        #f.write("\n\n\n") if n==1
        copy_psid_dat_be(f, dim1, dim2, dim3, 4)
      end
    end
  rescue => e
    print e
  ensure
    f.close
  end
end


# StructuredGrid のバイナリファイルを作成する。
# Scalars
#
def output_structured_grid_legacy_bin_with_scalars(filename, dim1, dim2, dim3, istep=0, isPd3d=true)
  begin
    f = File.open(filename, "wb")
    f.write("# vtk DataFile Version 2.0\n")
    f.write("PBVR sample GT5D - VTK binary\n")
    f.write("BINARY\n")
    f.write("DATASET STRUCTURED_GRID\n")
    f.write("DIMENSIONS %d %d %d\n" % [ dim1, dim2, dim3 ])
    f.write("POINTS %d float\n" % [ dim1*dim2*dim3 ])
    copy_co3d_dat_be(f, dim1, dim2, dim3, 0)           # 3*dim1*dim2*dim3 の float
    f.write("\n")
    f.write("POINT_DATA %d\n" % [ dim1*dim2*dim3 ])
    f.write("SCALARS scalars float 1\n")
    f.write("LOOKUP_TABLE default\n")
    if isPd3d then
      copy_pd3d_dat_be(f, dim1, dim2, dim3, 4, istep)
    else
      copy_psid_dat_be(f, dim1, dim2, dim3, 4)
    end
  rescue => e
    print e
  ensure
    f.close
  end
end


# RectilinearGrid のバイナリファイルを作成する。
# Scalars
#
def output_rectilinear_grid_legacy_bin_with_scalars(filename, dim1, dim2, dim3, istep=0, isPd3d=true)
  begin
    f = File.open(filename, "wb")
    f.write("# vtk DataFile Version 2.0\n")
    f.write("PBVR sample GT5D - VTK binary\n")
    f.write("BINARY\n")
    f.write("DATASET RECTILINEAR_GRID\n")
    f.write("DIMENSIONS %d %d %d\n" % [ dim1, dim2, dim3 ])
    f.write("X_COORDINATES %d float\n" % [ dim1 ])
    copy_axis_co3d_dat_be(f, dim1, dim2, dim3, 'x', 0, 0)
    f.write("Y_COORDINATES %d float\n" % [ dim2 ])
    copy_axis_co3d_dat_be(f, dim1, dim2, dim3, 'y', 0, 0)
    f.write("Z_COORDINATES %d float\n" % [ dim3 ])
    copy_axis_co3d_dat_be(f, dim1, dim2, dim3, 'z', 0, 0)
    f.write("\n")
    f.write("POINT_DATA %d\n" % [ dim1*dim2*dim3 ])
    f.write("SCALARS scalars float 1\n")
    f.write("LOOKUP_TABLE default\n")
    if isPd3d then
      copy_pd3d_dat_be(f, dim1, dim2, dim3, 4, istep)
    else
      copy_psid_dat_be(f, dim1, dim2, dim3, 4)
    end
  rescue => e
    print e
  ensure
    f.close
  end
end


###
### --------------------------------------------------------------------------------
###

if __FILE__ == $0
  dim1=220       # nx
  dim2=220       # ny
  dim3=48        # nz
  istep = 0      # 時間ステップ(0ベース)
  skipSize = 4
  
  isPd3d = false
  isChar = false
  
  minx = 169.83
  maxx = 388.83
  miny = -109.5
  maxy = 109.5
  minz = 0.0
  maxz = 345.0
  spacex = (maxx-minx) / dim1.to_f
  spacey = (maxy-miny) / dim2.to_f
  spacez = (maxz-minz) / dim3.to_f
  
  if false then                # STRUCTURED_POINTS
    if isPd3d then
      if isChar then
        filename = sprintf("gt5d_stpoints_bin_pd3d-vr-%05d.vtk", istep+1)
      else
        filename = sprintf("gt5d_stpoints_bin_pd3d-%05d.vtk", istep+1)
      end
    else
      if isChar then
        filename = "gt5d_stpoints_bin_psid-vr.vtk"
      else
        filename = "gt5d_stpoints_bin_psid.vtk"
      end
    end
    output_structured_points_legacy_bin_with_scalars(filename, dim1, dim2, dim3, spacex, spacey, spacez, istep, isPd3d, isChar)
  end
  
  if true then                # STRUCTURED_GRID
    if isPd3d then
      filename = sprintf("gt5d_stgrid_bin_pd3d-%05d.vtk", istep+1)
    else
      filename = "gt5d_stgrid_bin_psid.vtk"
    end
    output_structured_grid_legacy_bin_with_scalars(filename, dim1, dim2, dim3, istep, isPd3d)
  end
  
  if false then                # RECTILINEAR_GRID
    if isPd3d then
      filename = sprintf("gt5d_rectgrid_bin_pd3d-%05d.vtk", istep+1)
    else
      filename = "gt5d_rectgrid_bin_psid.vtk"
    end
    output_rectilinear_grid_legacy_bin_with_scalars(filename, dim1, dim2, dim3, istep, isPd3d)
  end
  
  if false then
    filename = sprintf("gt5d_stpoints_bin_field-%05d.vtk", istep+1)
    output_structured_points_vtk_bin_field(filename, dim1, dim2, dim3, spacex, spacey, spacez, 0)
  end
  
  #test_read_psid_dat(dim1, dim2, dim3, 4)
  #test_read_pd3d_dat(dim1, dim2, dim3, 4, 5)
  #fp = File.open("np.dat", "wb")
  #copy_psid_dat(fp, dim1, dim2, dim3, 4)
  #copy_pd3d_dat_unsigned_char(fp, dim1, dim2, dim3, skipSize, istep)
  #copy_psid_dat_unsigned_char(fp, dim1, dim2, dim3, skipSize)
  #fp.close
end

