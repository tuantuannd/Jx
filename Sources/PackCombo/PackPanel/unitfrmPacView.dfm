object frmPacView: TfrmPacView
  Left = 593
  Top = 188
  BorderStyle = bsDialog
  Caption = 'frmPacView'
  ClientHeight = 141
  ClientWidth = 341
  Color = clBtnFace
  Font.Charset = GB2312_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = #23435#20307
  Font.Style = []
  OldCreateOrder = False
  Position = poOwnerFormCenter
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
    Left = 8
    Top = 36
    Width = 48
    Height = 12
    Caption = #21103#29256#26412#65306
  end
  object alabPacFileName: TLabel
    Left = 8
    Top = 60
    Width = 48
    Height = 12
    Caption = #25991#20214#21517#65306
  end
  object alabPacFilePath: TLabel
    Left = 8
    Top = 84
    Width = 48
    Height = 12
    Caption = #36335#12288#24452#65306
  end
  object txtMainVer: TEdit
    Left = 56
    Top = 8
    Width = 73
    Height = 20
    Enabled = False
    ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
    TabOrder = 0
  end
  object txtSubVer: TEdit
    Left = 56
    Top = 32
    Width = 73
    Height = 20
    Enabled = False
    ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
    TabOrder = 1
  end
  object txtPacFileName: TEdit
    Left = 56
    Top = 56
    Width = 169
    Height = 20
    Enabled = False
    ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
    TabOrder = 2
    OnChange = txtMainTextChange
  end
  object txtPacFilePath: TEdit
    Left = 56
    Top = 80
    Width = 249
    Height = 20
    ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
    TabOrder = 3
    OnChange = txtMainTextChange
  end
  object btnPacFilePath: TButton
    Left = 311
    Top = 80
    Width = 25
    Height = 20
    Caption = '...'
    TabOrder = 4
    OnClick = btnPacFilePathClick
  end
  object btnClose: TButton
    Left = 256
    Top = 112
    Width = 75
    Height = 23
    Cancel = True
    Caption = #21462#28040'(&C)'
    TabOrder = 5
    OnClick = btnCloseClick
  end
  object btnOK: TButton
    Left = 168
    Top = 112
    Width = 75
    Height = 23
    Caption = #30830#23450'(&O)'
    Default = True
    Enabled = False
    TabOrder = 6
    OnClick = btnOKClick
  end
  object dlgOpen: TOpenDialog
    Filter = #28216#25103#36164#28304#21253'(*.pac)|*.pac'
    Left = 280
    Top = 24
  end
end
