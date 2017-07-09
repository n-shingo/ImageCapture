//
//  main.cpp
//  Opencv_VideoRec
//
//  Created by shingo on 2017/07/07.
//  Copyright © 2017年 shingo. All rights reserved.
//

#include <cstdio>
#include <unistd.h>
#include <string.h>
#include <chrono>
#include <sys/stat.h>
#include <opencv2/opencv.hpp>

#define NUM_PER_DIR 1000  // ディレクトリに対する枚数
//const char SAVE_ROOTDIR[] = "/Volumes/tsukuchare2017/VideoCapture";
const char FILE_PRENAME[] = "img";
const char DIR_PRENAME[] = "dir";
// 作成するディレクトリのパーミッション(全てrwx)
const mode_t DIR_PERMISSION = S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IROTH|S_IWOTH|S_IXOTH;

using namespace cv;

bool checkInterval( unsigned intvl_ms );
void imshow_ratio(const string& winname, const Mat& image, double ratio);
void showHelp(const char* arg0);


int main( int argc, char * argv[] ) {
    
    int cam_id = 0;
    char savefile_ext[8] = ".png";
    double zoomratio = 0.5;
    int autosave_intval_ms = 1000; // 自動保存のインターバル[ms]
    char save_rootdir[256] = "./";
    
    //
    // オプション解析
    //
    int opt;
    while( true )
    {
        bool first_nooption = true;
        opt = getopt(argc, argv,"c:e:z:i:h");

        if( opt != -1 ){
            switch(opt)
            {
                // カメラID
                case 'c':
                    cam_id = atoi(optarg);
                    break;
            
                // 保存ファイルの拡張子
                case 'e':
                    sprintf( savefile_ext, ".%s", optarg);
                    break;
                
                // 表示倍率
                case 'z':
                    zoomratio = atof(optarg);
                    break;
                
                // インターバル
                case 'i':
                    autosave_intval_ms = atoi(optarg);
                    break;
                
                // ヘルプ
                case 'h':
                    showHelp(argv[0]);
                    return 0;
            }
        }
        else{
            // オプション無しの引数1個目は保存ディレクトリ
            if( first_nooption && optind < argc )
                strcpy( save_rootdir, argv[optind] );
            
            optind++;
        }
        
        // 全ての引数の解析終了
        if( optind >= argc )
            break;
    }
    // <----オプション解析終了

    // 保存ルートディレクトリ整形
    if( save_rootdir[strlen(save_rootdir)-1] == '/' )
        save_rootdir[strlen(save_rootdir)-1] = '\0';

    // ルート保存ディレクトリの存在チェック
    struct stat statTmp;
    if( stat( save_rootdir, &statTmp) != 0 ){
        printf( "保存ルートディレクトリ %s が存在しません.\n", save_rootdir );
        printf( "プログラムを終了します.\n");
        showHelp( argv[0] );
        exit(1);
    }
    
    // 保存ディレクトリ(yyyymmdd_hhmmss)のパス文字列の作成
    char save_time_dir[128];
    time_t now = time(NULL);
    struct tm *pnow = localtime(&now);
    sprintf(save_time_dir, "%s/%04d%02d%02d_%02d%02d%02d", save_rootdir,
            pnow->tm_year + 1900, pnow->tm_mon + 1, pnow->tm_mday,
            pnow->tm_hour, pnow->tm_min, pnow->tm_sec);
    

    // ビデオキャプチャのオープン
    cv::VideoCapture cap = cv::VideoCapture(cam_id);
    if( !cap.isOpened() ){
        printf( "ビデオのオープンに失敗しました.\n" );
        printf( "プログラムを終了します.\n");
        showHelp( argv[0] );
        exit(1);
    }

    // ビデオキャプチャループ
    cv::Mat img;
    int img_save_num = 0;
    bool save_flag = false;
    bool auto_save = false;
    char current_save_dir[256];
    while(true)
    {
        // 画像の取得と表示
        cap >> img;
        imshow_ratio("Video", img, zoomratio);
        
        // キー入力
        int key = cv::waitKey(1);
        
        // 終了(QUIT)
        if( key == 'q' || key == 27 )
            break;
        
        // Manual保存(SAVE)
        if( key =='s' )
            save_flag = true;
        
        // Auto保存(AutoSave)
        if( key == 'a' ){
            auto_save = !auto_save;
            if( auto_save ) printf( "自動保存を開始します.(%dms 間隔)\n", autosave_intval_ms );
            else printf( "自動保存を停止します.\n" );
        }
        
        // Auto保存とインターバルチェック
        if( auto_save && checkInterval(autosave_intval_ms) )
            save_flag = true;

        // 画像を保存する
        if( save_flag ){
            
            // 保存ディレクトリ(yyyymmdd_hhmmss)の存在チェック
            if( stat( save_time_dir, &statTmp) != 0 ){
                if (mkdir(save_time_dir, DIR_PERMISSION ) == 0 )
                    printf("%s ディレクトリを生成しました.\n", save_time_dir );
                else{
                    printf("保存ディレクトリ %s の作成に失敗しました.\n", save_time_dir);
                    printf("プログラムを終了します.\n");
                    exit(1);
                }
            }
            
            // 保存ディレクトリ作成
            if( img_save_num % NUM_PER_DIR == 0)
            {
                sprintf( current_save_dir, "%s/%s%06d",save_time_dir, DIR_PRENAME, img_save_num );
                if( mkdir( current_save_dir, DIR_PERMISSION ) != 0 ){
                    printf("新規ディレクトリ %s の作成に失敗しました.\n", save_time_dir);
                    exit(1);
                }
            }
            
            // 保存ファイル名
            char filename[256];
            sprintf( filename, "%s/%s%06d%s", current_save_dir, FILE_PRENAME, img_save_num, savefile_ext);
            
            // 保存
            cv::imwrite( filename, img );
            printf( "%s を保存しました．\n", filename );

            // 後処理
            img_save_num++;
            save_flag = false;
        }
    }
    
    // opencv window の破棄
    cv::destroyAllWindows();
    
    // 終了
    printf( "プログラムを終了します.\n" );
    return 0;
}

// 指定のインターバルが経過したかチェックする
bool checkInterval( unsigned intvl_ms )
{
    // 前回スタンプからの経過時間計算
    static auto stamp_time = std::chrono::system_clock::now();
    auto now = std::chrono::system_clock::now();
    auto dur = now - stamp_time;
    auto intvl = std::chrono::milliseconds(intvl_ms);
    
    /*
    static time_t stamp_time = clock();
    clock_t now = clock();
    unsigned long dif_ms = 1000*(now-stamp_time) / CLOCKS_PER_SEC;
    */
    
    // インターバルを経過している
    if( dur >= intvl ) {
        stamp_time = now - (dur - intvl);
        return true;
    }
    
    // インターバルを経過していない
    return false;
}

// ratio サイズで画像を表示する
void imshow_ratio(const string& winname, const Mat& image, double ratio){
    Mat img_ratio = Mat();
    char win_name[64];
    cv::resize(image, img_ratio, cv::Size(0,0), ratio, ratio, cv::INTER_NEAREST);
    sprintf( win_name, "%s ( Zoom Ratio:%.3lf )",winname.c_str(), ratio);
    cv::imshow( win_name, img_ratio );
    img_ratio.release();
}

// ヘルプを表示する
void showHelp(const char* arg0)
{
    // arg0からファイル名だけを取り出す
    string exe_name = arg0;
    size_t slash_pos = exe_name.find_last_of('/');
    exe_name.erase(0, slash_pos+1);
    
    // 書式
    fprintf( stdout, "\n\n" );
    fprintf( stdout, "\033[1m書式\033[0m\n");
    fprintf( stdout, "\t\033[1m%s\033[0m [保存ディレクトリ] [-c カメラID] [-e 拡張子] [-z 表示倍率] [-i 保存間隔ms]\n", exe_name.c_str() );
    fprintf( stdout, "\t\033[1m%s\033[0m [-h]\n", exe_name.c_str() );
    fprintf( stdout, "\n" );
    
    // 説明
    fprintf( stdout, "\n" );
    fprintf( stdout, "\033[1m説明\033[0m\n" );
    fprintf( stdout, "\t\033[1m-i\033[0m\tカメラIDを指定する(初期値:0)\n" );
    fprintf( stdout, "\t\033[1m-e\033[0m\t保存する画像の拡張子を指定する(初期値:png)\n" );
    fprintf( stdout, "\t\033[1m-z\033[0m\t表示画像の倍率を指定する(初期値:0.5)\n" );
    fprintf( stdout, "\t\033[1m-i\033[0m\t自動保存のインターバルを指定する[ms](初期値:1000)\n" );
    fprintf( stdout, "\t\033[1m-h\033[0m\tこのヘルプを表示する\n" );
    fprintf( stdout, "\n" );
    
    // コマンド例
    fprintf( stdout, "\n" );
    fprintf( stdout, "\033[1mコマンド例\033[0m\n" );
    fprintf( stdout, "\t\033[1m%s\033[0m ~/tmp -c 1 -e jpg -z 0.25 -i 500\n\n", exe_name.c_str() );
    fprintf( stdout, "\tホームのtmpディレクトリに画像を保存する．カメラID=1の映像をjpg形式で保存し、\n");
    fprintf( stdout, "\t0.25のサイズで画面に表示する．自動保存の間隔は約500[ms]となる．\n");
    fprintf( stdout, "\n" );
    
    // 操作方法
    fprintf( stdout, "\n" );
    fprintf( stdout, "\033[1m操作方法\033[0m\n" );
    fprintf( stdout, "\t画像ウィンドウをアクティブにして以下のコマンドを入力して下さい．\n");
    fprintf( stdout, "\t\033[1ma\033[0m\t自動保存を開始／停止します．\n");
    fprintf( stdout, "\t\033[1ms\033[0m\t一枚だけをマニュアル保存します．\n");
    fprintf( stdout, "\t\033[1mq, ESC\033[0m\tアプリケーションを終了します．\n");
    
    // フッター
    fprintf( stdout, "\t\t\t\t\t\t\t2017年07月\n" );
    fprintf( stdout, "\n\n" );
    
    
}


