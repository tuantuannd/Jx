object frmMain: TfrmMain
  Left = 286
  Top = 166
  BorderStyle = bsDialog
  Caption = #21319#32423#25991#20214#29983#25104#22120
  ClientHeight = 385
  ClientWidth = 410
  Color = clBtnFace
  Font.Charset = GB2312_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = #23435#20307
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 12
  object alabMainVer: TLabel
    Left = 8
    Top = 12
    Width = 48
    Height = 12
    Caption = #20027#29256#26412#65306
  end
  object alabSubVer: TLabel
    Left = 128
    Top = 12
    Width = 48
    Height = 12
    Caption = #21103#29256#26412#65306
  end
  object alabWebDown: TLabel
    Left = 8
    Top = 44
    Width = 96
    Height = 12
    Caption = #20027#29256#26412#19979#36733#22320#22336#65306
  end
  object txtMainVer: TEdit
    Left = 56
    Top = 8
    Width = 57
    Height = 20
    ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
    TabOrder = 0
    Text = '0'
  end
  object txtSubVer: TEdit
    Left = 176
    Top = 8
    Width = 57
    Height = 20
    ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
    TabOrder = 1
    Text = '0'
  end
  object txtWebDown: TEdit
    Left = 104
    Top = 40
    Width = 297
    Height = 20
    ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
    TabOrder = 2
    Text = 'http://www.jxonline.net/dl_up.htm'
  end
  object gbSaveFile: TGroupBox
    Left = 8
    Top = 64
    Width = 393
    Height = 217
    Caption = #21319#32423#25152#38656#30340#25991#20214
    TabOrder = 3
    object btnOpenSaveFile: TButton
      Left = 8
      Top = 16
      Width = 89
      Height = 17
      Caption = #25171#24320#25991#20214'(&O)'
      TabOrder = 0
      OnClick = btnOpenSaveFileClick
    end
    object lvSaveFile: TListView
      Left = 8
      Top = 40
      Width = 377
      Height = 169
      Columns = <
        item
          Caption = #25991#20214#21517
          Width = 80
        end
        item
          Caption = #36335#24452
          Width = 80
        end
        item
          Caption = 'Belong'
        end
        item
          Caption = 'CheckVersionMethod'
          Width = 80
        end
        item
          Caption = 'UpdateFileFlag'
          Width = 80
        end
        item
          Caption = 'LocalPath'
          Width = 80
        end
        item
          Caption = 'UpdateFileMethod'
          Width = 80
        end
        item
          Caption = 'RemotePath'
          Width = 80
        end
        item
          Caption = 'UpdateRelative'
          Width = 80
        end>
      ColumnClick = False
      ReadOnly = True
      RowSelect = True
      TabOrder = 1
      ViewStyle = vsReport
      OnDblClick = lvSaveFileDblClick
    end
  end
  object gbSaveDir: TGroupBox
    Left = 8
    Top = 288
    Width = 393
    Height = 49
    Caption = #21319#32423#25991#20214#20445#23384#36335#24452
    TabOrder = 4
    object txtSaveDir: TEdit
      Left = 8
      Top = 16
      Width = 337
      Height = 20
      ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
      TabOrder = 0
      OnChange = txtSaveDirChange
    end
    object btnSaveDir: TButton
      Left = 352
      Top = 16
      Width = 25
      Height = 20
      Caption = '...'
      TabOrder = 1
      OnClick = btnSaveDirClick
    end
  end
  object btnRun: TButton
    Left = 216
    Top = 352
    Width = 75
    Height = 23
    Caption = #25191#34892'(&R)'
    Default = True
    Enabled = False
    TabOrder = 5
    OnClick = btnRunClick
  end
  object btnClose: TButton
    Left = 312
    Top = 352
    Width = 75
    Height = 23
    Cancel = True
    Caption = #20851#38381'(&C)'
    TabOrder = 6
    OnClick = btnCloseClick
  end
  object dlgOpen: TOpenDialog
    Filter = #25152#26377#25991#20214'(*.*)|*.*'
    Options = [ofHideReadOnly, ofAllowMultiSelect, ofEnableSizing]
    Left = 344
  end
end
