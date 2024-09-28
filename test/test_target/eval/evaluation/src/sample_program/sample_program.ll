; ModuleID = 'sample_program.c'
source_filename = "sample_program.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.benchmark_ctx = type { [256 x i8], i8, i8, i8* }

@key = internal constant [16 x i8] c"\0F\0E\0D\0C\0B\0A\09\08\07\06\05\04\03\02\01\00", align 16
@data = internal constant [16 x i8] c"\00\01\02\03\04\05\06\07\08\09\0A\0B\0C\0D\0E\0F", align 16
@result = internal global [16 x i8] zeroinitializer, align 16
@__const.verify_benchmark_sample_program.expected = private unnamed_addr constant [16 x i32] [i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15], align 16

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @scramble_memory(i8* %buffer, i32 %length) #0 {
entry:
  %buffer.addr = alloca i8*, align 8
  %length.addr = alloca i32, align 4
  %i = alloca i32, align 4
  store i8* %buffer, i8** %buffer.addr, align 8
  store i32 %length, i32* %length.addr, align 4
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, i32* %i, align 4
  %1 = load i32, i32* %length.addr, align 4
  %cmp = icmp slt i32 %0, %1
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %2 = load i8*, i8** %buffer.addr, align 8
  %3 = load i32, i32* %i, align 4
  %idxprom = sext i32 %3 to i64
  %arrayidx = getelementptr inbounds i8, i8* %2, i64 %idxprom
  %4 = load i8, i8* %arrayidx, align 1
  %conv = zext i8 %4 to i32
  %5 = load i32, i32* %i, align 4
  %add = add nsw i32 %conv, %5
  %and = and i32 %add, 255
  %6 = load i8*, i8** %buffer.addr, align 8
  %7 = load i32, i32* %i, align 4
  %idxprom1 = sext i32 %7 to i64
  %arrayidx2 = getelementptr inbounds i8, i8* %6, i64 %idxprom1
  %8 = load i8, i8* %arrayidx2, align 1
  %conv3 = zext i8 %8 to i32
  %xor = xor i32 %conv3, %and
  %conv4 = trunc i32 %xor to i8
  store i8 %conv4, i8* %arrayidx2, align 1
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %9 = load i32, i32* %i, align 4
  %inc = add nsw i32 %9, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond, !llvm.loop !4

for.end:                                          ; preds = %for.cond
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @setup_key(%struct.benchmark_ctx* %ctx, i8* %key, i32 %key_len) #0 {
entry:
  %ctx.addr = alloca %struct.benchmark_ctx*, align 8
  %key.addr = alloca i8*, align 8
  %key_len.addr = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %k = alloca i32, align 4
  %_t = alloca i32, align 4
  store %struct.benchmark_ctx* %ctx, %struct.benchmark_ctx** %ctx.addr, align 8
  store i8* %key, i8** %key.addr, align 8
  store i32 %key_len, i32* %key_len.addr, align 4
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, i32* %i, align 4
  %cmp = icmp ult i32 %0, 256
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %1 = load i32, i32* %i, align 4
  %conv = trunc i32 %1 to i8
  %2 = load %struct.benchmark_ctx*, %struct.benchmark_ctx** %ctx.addr, align 8
  %S = getelementptr inbounds %struct.benchmark_ctx, %struct.benchmark_ctx* %2, i32 0, i32 0
  %3 = load i32, i32* %i, align 4
  %idxprom = zext i32 %3 to i64
  %arrayidx = getelementptr inbounds [256 x i8], [256 x i8]* %S, i64 0, i64 %idxprom
  store i8 %conv, i8* %arrayidx, align 1
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %4 = load i32, i32* %i, align 4
  %inc = add i32 %4, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond, !llvm.loop !6

for.end:                                          ; preds = %for.cond
  store i32 0, i32* %k, align 4
  store i32 0, i32* %j, align 4
  store i32 0, i32* %i, align 4
  br label %for.cond1

for.cond1:                                        ; preds = %for.inc28, %for.end
  %5 = load i32, i32* %i, align 4
  %cmp2 = icmp ult i32 %5, 256
  br i1 %cmp2, label %for.body4, label %for.end30

for.body4:                                        ; preds = %for.cond1
  %6 = load %struct.benchmark_ctx*, %struct.benchmark_ctx** %ctx.addr, align 8
  %S5 = getelementptr inbounds %struct.benchmark_ctx, %struct.benchmark_ctx* %6, i32 0, i32 0
  %7 = load i32, i32* %i, align 4
  %idxprom6 = zext i32 %7 to i64
  %arrayidx7 = getelementptr inbounds [256 x i8], [256 x i8]* %S5, i64 0, i64 %idxprom6
  %8 = load i8, i8* %arrayidx7, align 1
  %conv8 = zext i8 %8 to i32
  %9 = load i8*, i8** %key.addr, align 8
  %10 = load i32, i32* %k, align 4
  %idxprom9 = zext i32 %10 to i64
  %arrayidx10 = getelementptr inbounds i8, i8* %9, i64 %idxprom9
  %11 = load i8, i8* %arrayidx10, align 1
  %conv11 = zext i8 %11 to i32
  %add = add nsw i32 %conv8, %conv11
  %12 = load i32, i32* %j, align 4
  %add12 = add i32 %12, %add
  store i32 %add12, i32* %j, align 4
  %13 = load i32, i32* %j, align 4
  %and = and i32 %13, 255
  store i32 %and, i32* %j, align 4
  br label %do.body

do.body:                                          ; preds = %for.body4
  %14 = load %struct.benchmark_ctx*, %struct.benchmark_ctx** %ctx.addr, align 8
  %S13 = getelementptr inbounds %struct.benchmark_ctx, %struct.benchmark_ctx* %14, i32 0, i32 0
  %15 = load i32, i32* %i, align 4
  %idxprom14 = zext i32 %15 to i64
  %arrayidx15 = getelementptr inbounds [256 x i8], [256 x i8]* %S13, i64 0, i64 %idxprom14
  %16 = load i8, i8* %arrayidx15, align 1
  %conv16 = zext i8 %16 to i32
  store i32 %conv16, i32* %_t, align 4
  %17 = load %struct.benchmark_ctx*, %struct.benchmark_ctx** %ctx.addr, align 8
  %S17 = getelementptr inbounds %struct.benchmark_ctx, %struct.benchmark_ctx* %17, i32 0, i32 0
  %18 = load i32, i32* %j, align 4
  %idxprom18 = zext i32 %18 to i64
  %arrayidx19 = getelementptr inbounds [256 x i8], [256 x i8]* %S17, i64 0, i64 %idxprom18
  %19 = load i8, i8* %arrayidx19, align 1
  %20 = load %struct.benchmark_ctx*, %struct.benchmark_ctx** %ctx.addr, align 8
  %S20 = getelementptr inbounds %struct.benchmark_ctx, %struct.benchmark_ctx* %20, i32 0, i32 0
  %21 = load i32, i32* %i, align 4
  %idxprom21 = zext i32 %21 to i64
  %arrayidx22 = getelementptr inbounds [256 x i8], [256 x i8]* %S20, i64 0, i64 %idxprom21
  store i8 %19, i8* %arrayidx22, align 1
  %22 = load i32, i32* %_t, align 4
  %conv23 = trunc i32 %22 to i8
  %23 = load %struct.benchmark_ctx*, %struct.benchmark_ctx** %ctx.addr, align 8
  %S24 = getelementptr inbounds %struct.benchmark_ctx, %struct.benchmark_ctx* %23, i32 0, i32 0
  %24 = load i32, i32* %j, align 4
  %idxprom25 = zext i32 %24 to i64
  %arrayidx26 = getelementptr inbounds [256 x i8], [256 x i8]* %S24, i64 0, i64 %idxprom25
  store i8 %conv23, i8* %arrayidx26, align 1
  br label %do.end

do.end:                                           ; preds = %do.body
  %25 = load i32, i32* %k, align 4
  %add27 = add i32 %25, 1
  %26 = load i32, i32* %key_len.addr, align 4
  %rem = urem i32 %add27, %26
  store i32 %rem, i32* %k, align 4
  br label %for.inc28

for.inc28:                                        ; preds = %do.end
  %27 = load i32, i32* %i, align 4
  %inc29 = add i32 %27, 1
  store i32 %inc29, i32* %i, align 4
  br label %for.cond1, !llvm.loop !7

for.end30:                                        ; preds = %for.cond1
  %28 = load %struct.benchmark_ctx*, %struct.benchmark_ctx** %ctx.addr, align 8
  %j31 = getelementptr inbounds %struct.benchmark_ctx, %struct.benchmark_ctx* %28, i32 0, i32 2
  store i8 0, i8* %j31, align 1
  %29 = load %struct.benchmark_ctx*, %struct.benchmark_ctx** %ctx.addr, align 8
  %i32 = getelementptr inbounds %struct.benchmark_ctx, %struct.benchmark_ctx* %29, i32 0, i32 1
  store i8 0, i8* %i32, align 8
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @perform_crypt(%struct.benchmark_ctx* %ctx, i32 %length, i8* %dst, i8* %src) #0 {
entry:
  %ctx.addr = alloca %struct.benchmark_ctx*, align 8
  %length.addr = alloca i32, align 4
  %dst.addr = alloca i8*, align 8
  %src.addr = alloca i8*, align 8
  %i = alloca i8, align 1
  %j = alloca i8, align 1
  %si = alloca i32, align 4
  %sj = alloca i32, align 4
  store %struct.benchmark_ctx* %ctx, %struct.benchmark_ctx** %ctx.addr, align 8
  store i32 %length, i32* %length.addr, align 4
  store i8* %dst, i8** %dst.addr, align 8
  store i8* %src, i8** %src.addr, align 8
  %0 = load %struct.benchmark_ctx*, %struct.benchmark_ctx** %ctx.addr, align 8
  %i1 = getelementptr inbounds %struct.benchmark_ctx, %struct.benchmark_ctx* %0, i32 0, i32 1
  %1 = load i8, i8* %i1, align 8
  store i8 %1, i8* %i, align 1
  %2 = load %struct.benchmark_ctx*, %struct.benchmark_ctx** %ctx.addr, align 8
  %j2 = getelementptr inbounds %struct.benchmark_ctx, %struct.benchmark_ctx* %2, i32 0, i32 2
  %3 = load i8, i8* %j2, align 1
  store i8 %3, i8* %j, align 1
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %4 = load i32, i32* %length.addr, align 4
  %dec = add nsw i32 %4, -1
  store i32 %dec, i32* %length.addr, align 4
  %tobool = icmp ne i32 %4, 0
  br i1 %tobool, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  %5 = load i8, i8* %i, align 1
  %inc = add i8 %5, 1
  store i8 %inc, i8* %i, align 1
  %6 = load i8, i8* %i, align 1
  %conv = zext i8 %6 to i32
  %and = and i32 %conv, 255
  %conv3 = trunc i32 %and to i8
  store i8 %conv3, i8* %i, align 1
  %7 = load %struct.benchmark_ctx*, %struct.benchmark_ctx** %ctx.addr, align 8
  %S = getelementptr inbounds %struct.benchmark_ctx, %struct.benchmark_ctx* %7, i32 0, i32 0
  %8 = load i8, i8* %i, align 1
  %idxprom = zext i8 %8 to i64
  %arrayidx = getelementptr inbounds [256 x i8], [256 x i8]* %S, i64 0, i64 %idxprom
  %9 = load i8, i8* %arrayidx, align 1
  %conv4 = zext i8 %9 to i32
  store i32 %conv4, i32* %si, align 4
  %10 = load i32, i32* %si, align 4
  %11 = load i8, i8* %j, align 1
  %conv5 = zext i8 %11 to i32
  %add = add nsw i32 %conv5, %10
  %conv6 = trunc i32 %add to i8
  store i8 %conv6, i8* %j, align 1
  %12 = load i8, i8* %j, align 1
  %conv7 = zext i8 %12 to i32
  %and8 = and i32 %conv7, 255
  %conv9 = trunc i32 %and8 to i8
  store i8 %conv9, i8* %j, align 1
  %13 = load %struct.benchmark_ctx*, %struct.benchmark_ctx** %ctx.addr, align 8
  %S10 = getelementptr inbounds %struct.benchmark_ctx, %struct.benchmark_ctx* %13, i32 0, i32 0
  %14 = load i8, i8* %j, align 1
  %idxprom11 = zext i8 %14 to i64
  %arrayidx12 = getelementptr inbounds [256 x i8], [256 x i8]* %S10, i64 0, i64 %idxprom11
  %15 = load i8, i8* %arrayidx12, align 1
  %16 = load %struct.benchmark_ctx*, %struct.benchmark_ctx** %ctx.addr, align 8
  %S13 = getelementptr inbounds %struct.benchmark_ctx, %struct.benchmark_ctx* %16, i32 0, i32 0
  %17 = load i8, i8* %i, align 1
  %idxprom14 = zext i8 %17 to i64
  %arrayidx15 = getelementptr inbounds [256 x i8], [256 x i8]* %S13, i64 0, i64 %idxprom14
  store i8 %15, i8* %arrayidx15, align 1
  %conv16 = zext i8 %15 to i32
  store i32 %conv16, i32* %sj, align 4
  %18 = load i32, i32* %si, align 4
  %conv17 = trunc i32 %18 to i8
  %19 = load %struct.benchmark_ctx*, %struct.benchmark_ctx** %ctx.addr, align 8
  %S18 = getelementptr inbounds %struct.benchmark_ctx, %struct.benchmark_ctx* %19, i32 0, i32 0
  %20 = load i8, i8* %j, align 1
  %idxprom19 = zext i8 %20 to i64
  %arrayidx20 = getelementptr inbounds [256 x i8], [256 x i8]* %S18, i64 0, i64 %idxprom19
  store i8 %conv17, i8* %arrayidx20, align 1
  %21 = load i8*, i8** %src.addr, align 8
  %incdec.ptr = getelementptr inbounds i8, i8* %21, i32 1
  store i8* %incdec.ptr, i8** %src.addr, align 8
  %22 = load i8, i8* %21, align 1
  %conv21 = zext i8 %22 to i32
  %23 = load %struct.benchmark_ctx*, %struct.benchmark_ctx** %ctx.addr, align 8
  %S22 = getelementptr inbounds %struct.benchmark_ctx, %struct.benchmark_ctx* %23, i32 0, i32 0
  %24 = load i32, i32* %si, align 4
  %25 = load i32, i32* %sj, align 4
  %add23 = add nsw i32 %24, %25
  %and24 = and i32 %add23, 255
  %idxprom25 = sext i32 %and24 to i64
  %arrayidx26 = getelementptr inbounds [256 x i8], [256 x i8]* %S22, i64 0, i64 %idxprom25
  %26 = load i8, i8* %arrayidx26, align 1
  %conv27 = zext i8 %26 to i32
  %xor = xor i32 %conv21, %conv27
  %conv28 = trunc i32 %xor to i8
  %27 = load i8*, i8** %dst.addr, align 8
  %incdec.ptr29 = getelementptr inbounds i8, i8* %27, i32 1
  store i8* %incdec.ptr29, i8** %dst.addr, align 8
  store i8 %conv28, i8* %27, align 1
  br label %while.cond, !llvm.loop !8

while.end:                                        ; preds = %while.cond
  %28 = load i8, i8* %i, align 1
  %29 = load %struct.benchmark_ctx*, %struct.benchmark_ctx** %ctx.addr, align 8
  %i30 = getelementptr inbounds %struct.benchmark_ctx, %struct.benchmark_ctx* %29, i32 0, i32 1
  store i8 %28, i8* %i30, align 8
  %30 = load i8, i8* %j, align 1
  %31 = load %struct.benchmark_ctx*, %struct.benchmark_ctx** %ctx.addr, align 8
  %j31 = getelementptr inbounds %struct.benchmark_ctx, %struct.benchmark_ctx* %31, i32 0, i32 2
  store i8 %30, i8* %j31, align 1
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @modify_context(%struct.benchmark_ctx* %ctx, i32 %offset, i32 %length) #0 {
entry:
  %ctx.addr = alloca %struct.benchmark_ctx*, align 8
  %offset.addr = alloca i32, align 4
  %length.addr = alloca i32, align 4
  %i = alloca i32, align 4
  store %struct.benchmark_ctx* %ctx, %struct.benchmark_ctx** %ctx.addr, align 8
  store i32 %offset, i32* %offset.addr, align 4
  store i32 %length, i32* %length.addr, align 4
  %0 = load i32, i32* %offset.addr, align 4
  store i32 %0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %1 = load i32, i32* %i, align 4
  %2 = load i32, i32* %offset.addr, align 4
  %3 = load i32, i32* %length.addr, align 4
  %add = add nsw i32 %2, %3
  %cmp = icmp slt i32 %1, %add
  br i1 %cmp, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %for.cond
  %4 = load i32, i32* %i, align 4
  %cmp1 = icmp slt i32 %4, 256
  br label %land.end

land.end:                                         ; preds = %land.rhs, %for.cond
  %5 = phi i1 [ false, %for.cond ], [ %cmp1, %land.rhs ]
  br i1 %5, label %for.body, label %for.end

for.body:                                         ; preds = %land.end
  %6 = load %struct.benchmark_ctx*, %struct.benchmark_ctx** %ctx.addr, align 8
  %S = getelementptr inbounds %struct.benchmark_ctx, %struct.benchmark_ctx* %6, i32 0, i32 0
  %7 = load i32, i32* %i, align 4
  %idxprom = sext i32 %7 to i64
  %arrayidx = getelementptr inbounds [256 x i8], [256 x i8]* %S, i64 0, i64 %idxprom
  %8 = load i8, i8* %arrayidx, align 1
  %conv = zext i8 %8 to i32
  %9 = load i32, i32* %length.addr, align 4
  %add2 = add nsw i32 %conv, %9
  %and = and i32 %add2, 255
  %10 = load %struct.benchmark_ctx*, %struct.benchmark_ctx** %ctx.addr, align 8
  %S3 = getelementptr inbounds %struct.benchmark_ctx, %struct.benchmark_ctx* %10, i32 0, i32 0
  %11 = load i32, i32* %i, align 4
  %idxprom4 = sext i32 %11 to i64
  %arrayidx5 = getelementptr inbounds [256 x i8], [256 x i8]* %S3, i64 0, i64 %idxprom4
  %12 = load i8, i8* %arrayidx5, align 1
  %conv6 = zext i8 %12 to i32
  %xor = xor i32 %conv6, %and
  %conv7 = trunc i32 %xor to i8
  store i8 %conv7, i8* %arrayidx5, align 1
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %13 = load i32, i32* %i, align 4
  %inc = add nsw i32 %13, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond, !llvm.loop !9

for.end:                                          ; preds = %land.end
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @extra_processing(i8* %data_ptr, %struct.benchmark_ctx* %ctx_ptr, i32 %length) #0 {
entry:
  %data_ptr.addr = alloca i8*, align 8
  %ctx_ptr.addr = alloca %struct.benchmark_ctx*, align 8
  %length.addr = alloca i32, align 4
  %i = alloca i32, align 4
  store i8* %data_ptr, i8** %data_ptr.addr, align 8
  store %struct.benchmark_ctx* %ctx_ptr, %struct.benchmark_ctx** %ctx_ptr.addr, align 8
  store i32 %length, i32* %length.addr, align 4
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, i32* %i, align 4
  %1 = load i32, i32* %length.addr, align 4
  %cmp = icmp slt i32 %0, %1
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %2 = load i8*, i8** %data_ptr.addr, align 8
  %3 = load i32, i32* %i, align 4
  %idxprom = sext i32 %3 to i64
  %arrayidx = getelementptr inbounds i8, i8* %2, i64 %idxprom
  %4 = load i8, i8* %arrayidx, align 1
  %conv = zext i8 %4 to i32
  %5 = load %struct.benchmark_ctx*, %struct.benchmark_ctx** %ctx_ptr.addr, align 8
  %S = getelementptr inbounds %struct.benchmark_ctx, %struct.benchmark_ctx* %5, i32 0, i32 0
  %6 = load i32, i32* %i, align 4
  %idxprom1 = sext i32 %6 to i64
  %arrayidx2 = getelementptr inbounds [256 x i8], [256 x i8]* %S, i64 0, i64 %idxprom1
  %7 = load i8, i8* %arrayidx2, align 1
  %conv3 = zext i8 %7 to i32
  %xor = xor i32 %conv, %conv3
  %and = and i32 %xor, 255
  %conv4 = trunc i32 %and to i8
  %8 = load i8*, i8** %data_ptr.addr, align 8
  %9 = load i32, i32* %i, align 4
  %idxprom5 = sext i32 %9 to i64
  %arrayidx6 = getelementptr inbounds i8, i8* %8, i64 %idxprom5
  store i8 %conv4, i8* %arrayidx6, align 1
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %10 = load i32, i32* %i, align 4
  %inc = add nsw i32 %10, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond, !llvm.loop !10

for.end:                                          ; preds = %for.cond
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @initialise_benchmark_sample_program() #0 {
entry:
  %ctx = alloca %struct.benchmark_ctx*, align 8
  %i = alloca i32, align 4
  %call = call noalias align 16 i8* @malloc(i64 272) #3
  %0 = bitcast i8* %call to %struct.benchmark_ctx*
  store %struct.benchmark_ctx* %0, %struct.benchmark_ctx** %ctx, align 8
  %call1 = call noalias align 16 i8* @malloc(i64 16) #3
  %1 = load %struct.benchmark_ctx*, %struct.benchmark_ctx** %ctx, align 8
  %extra_data = getelementptr inbounds %struct.benchmark_ctx, %struct.benchmark_ctx* %1, i32 0, i32 3
  store i8* %call1, i8** %extra_data, align 8
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %2 = load i32, i32* %i, align 4
  %cmp = icmp slt i32 %2, 16
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %3 = load i32, i32* %i, align 4
  %idxprom = sext i32 %3 to i64
  %arrayidx = getelementptr inbounds [16 x i8], [16 x i8]* @key, i64 0, i64 %idxprom
  %4 = load i8, i8* %arrayidx, align 1
  %conv = zext i8 %4 to i32
  %5 = load i32, i32* %i, align 4
  %idxprom2 = sext i32 %5 to i64
  %arrayidx3 = getelementptr inbounds [16 x i8], [16 x i8]* @data, i64 0, i64 %idxprom2
  %6 = load i8, i8* %arrayidx3, align 1
  %conv4 = zext i8 %6 to i32
  %add = add nsw i32 %conv, %conv4
  %and = and i32 %add, 255
  %conv5 = trunc i32 %and to i8
  %7 = load %struct.benchmark_ctx*, %struct.benchmark_ctx** %ctx, align 8
  %extra_data6 = getelementptr inbounds %struct.benchmark_ctx, %struct.benchmark_ctx* %7, i32 0, i32 3
  %8 = load i8*, i8** %extra_data6, align 8
  %9 = load i32, i32* %i, align 4
  %idxprom7 = sext i32 %9 to i64
  %arrayidx8 = getelementptr inbounds i8, i8* %8, i64 %idxprom7
  store i8 %conv5, i8* %arrayidx8, align 1
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %10 = load i32, i32* %i, align 4
  %inc = add nsw i32 %10, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond, !llvm.loop !11

for.end:                                          ; preds = %for.cond
  %11 = load %struct.benchmark_ctx*, %struct.benchmark_ctx** %ctx, align 8
  %extra_data9 = getelementptr inbounds %struct.benchmark_ctx, %struct.benchmark_ctx* %11, i32 0, i32 3
  %12 = load i8*, i8** %extra_data9, align 8
  call void @free(i8* %12) #3
  %13 = load %struct.benchmark_ctx*, %struct.benchmark_ctx** %ctx, align 8
  %14 = bitcast %struct.benchmark_ctx* %13 to i8*
  call void @free(i8* %14) #3
  ret void
}

; Function Attrs: nounwind
declare dso_local noalias align 16 i8* @malloc(i64) #1

; Function Attrs: nounwind
declare dso_local void @free(i8*) #1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @benchmark_sample_program() #0 {
entry:
  %ctx = alloca %struct.benchmark_ctx*, align 8
  %call = call noalias align 16 i8* @malloc(i64 272) #3
  %0 = bitcast i8* %call to %struct.benchmark_ctx*
  store %struct.benchmark_ctx* %0, %struct.benchmark_ctx** %ctx, align 8
  %call1 = call noalias align 16 i8* @malloc(i64 16) #3
  %1 = load %struct.benchmark_ctx*, %struct.benchmark_ctx** %ctx, align 8
  %extra_data = getelementptr inbounds %struct.benchmark_ctx, %struct.benchmark_ctx* %1, i32 0, i32 3
  store i8* %call1, i8** %extra_data, align 8
  %2 = load %struct.benchmark_ctx*, %struct.benchmark_ctx** %ctx, align 8
  call void @setup_key(%struct.benchmark_ctx* %2, i8* getelementptr inbounds ([16 x i8], [16 x i8]* @key, i64 0, i64 0), i32 16)
  %3 = load %struct.benchmark_ctx*, %struct.benchmark_ctx** %ctx, align 8
  call void @perform_crypt(%struct.benchmark_ctx* %3, i32 16, i8* getelementptr inbounds ([16 x i8], [16 x i8]* @result, i64 0, i64 0), i8* getelementptr inbounds ([16 x i8], [16 x i8]* @data, i64 0, i64 0))
  %4 = load %struct.benchmark_ctx*, %struct.benchmark_ctx** %ctx, align 8
  call void @modify_context(%struct.benchmark_ctx* %4, i32 5, i32 10)
  %5 = load %struct.benchmark_ctx*, %struct.benchmark_ctx** %ctx, align 8
  call void @extra_processing(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @result, i64 0, i64 0), %struct.benchmark_ctx* %5, i32 16)
  %6 = load %struct.benchmark_ctx*, %struct.benchmark_ctx** %ctx, align 8
  call void @setup_key(%struct.benchmark_ctx* %6, i8* getelementptr inbounds ([16 x i8], [16 x i8]* @key, i64 0, i64 0), i32 16)
  %7 = load %struct.benchmark_ctx*, %struct.benchmark_ctx** %ctx, align 8
  call void @perform_crypt(%struct.benchmark_ctx* %7, i32 16, i8* getelementptr inbounds ([16 x i8], [16 x i8]* @result, i64 0, i64 0), i8* getelementptr inbounds ([16 x i8], [16 x i8]* @result, i64 0, i64 0))
  %8 = load %struct.benchmark_ctx*, %struct.benchmark_ctx** %ctx, align 8
  %extra_data2 = getelementptr inbounds %struct.benchmark_ctx, %struct.benchmark_ctx* %8, i32 0, i32 3
  %9 = load i8*, i8** %extra_data2, align 8
  call void @free(i8* %9) #3
  %10 = load %struct.benchmark_ctx*, %struct.benchmark_ctx** %ctx, align 8
  %11 = bitcast %struct.benchmark_ctx* %10 to i8*
  call void @free(i8* %11) #3
  ret i32 0
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @verify_benchmark_sample_program(i32 %unused) #0 {
entry:
  %retval = alloca i32, align 4
  %unused.addr = alloca i32, align 4
  %expected = alloca [16 x i32], align 16
  %i = alloca i32, align 4
  store i32 %unused, i32* %unused.addr, align 4
  %0 = bitcast [16 x i32]* %expected to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 16 %0, i8* align 16 bitcast ([16 x i32]* @__const.verify_benchmark_sample_program.expected to i8*), i64 64, i1 false)
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %1 = load i32, i32* %i, align 4
  %cmp = icmp slt i32 %1, 16
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %2 = load i32, i32* %i, align 4
  %idxprom = sext i32 %2 to i64
  %arrayidx = getelementptr inbounds [16 x i8], [16 x i8]* @result, i64 0, i64 %idxprom
  %3 = load i8, i8* %arrayidx, align 1
  %conv = zext i8 %3 to i32
  %4 = load i32, i32* %i, align 4
  %idxprom1 = sext i32 %4 to i64
  %arrayidx2 = getelementptr inbounds [16 x i32], [16 x i32]* %expected, i64 0, i64 %idxprom1
  %5 = load i32, i32* %arrayidx2, align 4
  %cmp3 = icmp ne i32 %conv, %5
  br i1 %cmp3, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  store i32 0, i32* %retval, align 4
  br label %return

if.end:                                           ; preds = %for.body
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %6 = load i32, i32* %i, align 4
  %inc = add nsw i32 %6, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond, !llvm.loop !12

for.end:                                          ; preds = %for.cond
  store i32 1, i32* %retval, align 4
  br label %return

return:                                           ; preds = %for.end, %if.then
  %7 = load i32, i32* %retval, align 4
  ret i32 %7
}

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #2

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nounwind "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { argmemonly nofree nounwind willreturn }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{i32 7, !"frame-pointer", i32 2}
!3 = !{!"clang version 13.0.1 (https://github.com/llvm/llvm-project.git 75e33f71c2dae584b13a7d1186ae0a038ba98838)"}
!4 = distinct !{!4, !5}
!5 = !{!"llvm.loop.mustprogress"}
!6 = distinct !{!6, !5}
!7 = distinct !{!7, !5}
!8 = distinct !{!8, !5}
!9 = distinct !{!9, !5}
!10 = distinct !{!10, !5}
!11 = distinct !{!11, !5}
!12 = distinct !{!12, !5}
