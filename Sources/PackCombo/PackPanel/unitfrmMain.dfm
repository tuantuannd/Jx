object frmMain: TfrmMain
  Left = 361
  Top = 213
  BorderStyle = bsDialog
  Caption = #28216#25103#26356#26032#21253#25511#21046#21488
  ClientHeight = 402
  ClientWidth = 607
  Color = clBtnFace
  Font.Charset = GB2312_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = #23435#20307
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 12
  object tabMain: TPageControl
    Left = 8
    Top = 8
    Width = 593
    Height = 385
    ActivePage = tabsViewPac
    TabOrder = 0
    object tabsViewPac: TTabSheet
      Caption = #36164#28304#21253#26597#30475
      object alabMainVer: TLabel
        Left = 8
        Top = 12
        Width = 48
        Height = 12
        Caption = #20027#29256#26412#65306
      end
      object gbSubPack: TGroupBox
        Left = 8
        Top = 112
        Width = 569
        Height = 241
        Caption = #21103#29256#26412#36164#28304#21253
        TabOrder = 0
        object lvSubPack: TListView
          Left = 8
          Top = 40
          Width = 553
          Height = 193
          Columns = <
            item
              Caption = #25991#20214#21517
              Width = 130
            end
            item
              Caption = #20027#29256#26412
            end
            item
              Caption = #21103#29256#26412
            end
            item
              Caption = #22823#23567
              Width = 80
            end
            item
              Caption = #36335#24452
              Width = 200
            end>
          ReadOnly = True
          RowSelect = True
          TabOrder = 0
          ViewStyle = vsReport
          OnColumnClick = lvSubPackColumnClick
          OnDblClick = lvSubPackDblClick
          OnSelectItem = lvSubPackSelectItem
        end
        object btnSubVerBuild: TButton
          Left = 8
          Top = 16
          Width = 97
          Height = 20
          Caption = #26032#24314#21103#29256#26412'(&S)'
          TabOrder = 1
          OnClick = btnSubVerBuildClick
        end
        object btnMakeUpdate: TButton
          Left = 448
          Top = 16
          Width = 113
          Height = 20
          Caption = #29983#25104#21319#32423#21253#24207#21015'(&C)'
          Enabled = False
          TabOrder = 2
          OnClick = btnMakeUpdateClick
        end
        object btnSubVerDel: TButton
          Left = 232
          Top = 16
          Width = 97
          Height = 20
          Caption = #21024#38500#21103#29256#26412'(&K)'
          Enabled = False
          TabOrder = 3
          OnClick = btnSubVerDelClick
        end
        object btnSubVerMod: TButton
          Left = 120
          Top = 16
          Width = 97
          Height = 20
          Caption = #20462#25913#21103#29256#26412'(&P)'
          Enabled = False
          TabOrder = 4
          OnClick = btnSubVerModClick
        end
      end
      object gbMainPack: TGroupBox
        Left = 8
        Top = 32
        Width = 569
        Height = 73
        Caption = #20027#29256#26412#36164#28304#21253
        TabOrder = 1
        object alabMainPackSize: TLabel
          Left = 232
          Top = 24
          Width = 60
          Height = 12
          Caption = #25991#20214#22823#23567#65306
        end
        object labMainPackSize: TLabel
          Left = 296
          Top = 24
          Width = 6
          Height = 12
        end
        object alabMainPackName: TLabel
          Left = 16
          Top = 24
          Width = 48
          Height = 12
          Caption = #25991#20214#21517#65306
        end
        object labMainPack: TLabel
          Left = 64
          Top = 24
          Width = 6
          Height = 12
        end
        object alabMainPack: TLabel
          Left = 28
          Top = 46
          Width = 36
          Height = 12
          Caption = #36335#24452#65306
        end
        object txtMainPack: TEdit
          Left = 72
          Top = 42
          Width = 481
          Height = 20
          ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
          ReadOnly = True
          TabOrder = 0
        end
      end
      object cbMainVer: TComboBox
        Left = 56
        Top = 8
        Width = 105
        Height = 20
        Style = csDropDownList
        ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
        ItemHeight = 12
        TabOrder = 2
        OnChange = cbMainVerChange
      end
      object btnMainVerBuild: TButton
        Left = 176
        Top = 8
        Width = 97
        Height = 20
        Caption = #26032#24314#20027#29256#26412'(&M)'
        TabOrder = 3
        OnClick = btnMainVerBuildClick
      end
      object btnMainVerDel: TButton
        Left = 400
        Top = 8
        Width = 97
        Height = 20
        Caption = #21024#38500#20027#29256#26412'(&D)'
        TabOrder = 4
        OnClick = btnMainVerDelClick
      end
      object btnMainVerMod: TButton
        Left = 288
        Top = 8
        Width = 97
        Height = 20
        Caption = #20462#25913#20027#29256#26412'(&O)'
        TabOrder = 5
        OnClick = btnMainVerModClick
      end
    end
    object tabsViewUpdate: TTabSheet
      Caption = #26356#26032#21253#26597#30475
      ImageIndex = 1
      object alabUpateMainVer: TLabel
        Left = 8
        Top = 12
        Width = 48
        Height = 12
        Caption = #20027#29256#26412#65306
      end
      object alabUpateSubVer: TLabel
        Left = 176
        Top = 12
        Width = 48
        Height = 12
        Caption = #21103#29256#26412#65306
      end
      object cbUpdateMainVer: TComboBox
        Left = 56
        Top = 8
        Width = 105
        Height = 20
        Style = csDropDownList
        ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
        ItemHeight = 12
        TabOrder = 0
        OnChange = cbUpdateVerChange
      end
      object lvUpdatePack: TListView
        Left = 8
        Top = 32
        Width = 569
        Height = 297
        Columns = <
          item
            Caption = #25991#20214#21517
            Width = 130
          end
          item
            Caption = #20027#29256#26412
          end
          item
            Caption = #29256#26412':'#20174
            Width = 60
          end
          item
            Caption = #29256#26412':'#21040
            Width = 60
          end
          item
            Caption = #22823#23567
            Width = 80
          end
          item
            Caption = #36335#24452
            Width = 175
          end>
        ReadOnly = True
        RowSelect = True
        TabOrder = 1
        ViewStyle = vsReport
        OnColumnClick = lvUpdatePackColumnClick
        OnDblClick = lvUpdatePackDblClick
        OnSelectItem = lvUpdatePackSelectItem
      end
      object btnUpdateDel: TButton
        Left = 112
        Top = 336
        Width = 97
        Height = 20
        Caption = #21024#38500#21319#32423#21253'(&D)'
        Enabled = False
        TabOrder = 2
        OnClick = btnUpdateDelClick
      end
      object btnUpdateMod: TButton
        Left = 8
        Top = 336
        Width = 97
        Height = 20
        Caption = #20462#25913#26356#26032#21253'(&P)'
        Enabled = False
        TabOrder = 3
        OnClick = btnUpdateModClick
      end
      object cbUpdateSubVer: TComboBox
        Left = 224
        Top = 8
        Width = 105
        Height = 20
        Style = csDropDownList
        ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
        ItemHeight = 12
        TabOrder = 4
        OnChange = cbUpdateSubVerChange
      end
      object btnUpdateCopyToMake: TButton
        Left = 416
        Top = 8
        Width = 161
        Height = 20
        Caption = #22797#21046#21040#21319#32423#25991#20214#29983#25104#22120'(&C)'
        TabOrder = 5
        OnClick = btnUpdateCopyToMakeClick
      end
    end
    object tabsViewMake: TTabSheet
      Caption = #21319#32423#25991#20214#29983#25104#22120
      ImageIndex = 2
      object alabMakeMainVer: TLabel
        Left = 8
        Top = 12
        Width = 48
        Height = 12
        Caption = #20027#29256#26412#65306
      end
      object alabMakeSubVer: TLabel
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
      object txtMakeMainVer: TEdit
        Left = 56
        Top = 8
        Width = 57
        Height = 20
        ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
        TabOrder = 0
        Text = '0'
        OnChange = txtMakeTextChange
      end
      object txtMakeSubVer: TEdit
        Left = 176
        Top = 8
        Width = 57
        Height = 20
        ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
        TabOrder = 1
        Text = '0'
        OnChange = txtMakeTextChange
      end
      object txtMakeWebDown: TEdit
        Left = 104
        Top = 40
        Width = 297
        Height = 20
        ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
        TabOrder = 2
        Text = 'http://www.jxonline.net/dl_up.htm'
      end
      object gbMakeSaveFile: TGroupBox
        Left = 8
        Top = 64
        Width = 569
        Height = 233
        Caption = #21319#32423#25152#38656#30340#25991#20214
        TabOrder = 3
        object btnMakeOpenSaveFile: TButton
          Left = 8
          Top = 16
          Width = 89
          Height = 17
          Caption = #25171#24320#25991#20214'(&O)'
          TabOrder = 0
          OnClick = btnMakeOpenSaveFileClick
        end
        object lvMakeSaveFile: TListView
          Left = 8
          Top = 40
          Width = 553
          Height = 185
          Columns = <
            item
              Caption = #25991#20214#21517
              Width = 100
            end
            item
              Caption = #36335#24452
              Width = 100
            end
            item
              Caption = 'Belong'
            end
            item
              Caption = 'CheckVersionMethod'
              Width = 100
            end
            item
              Caption = 'UpdateFileFlag'
              Width = 100
            end
            item
              Caption = 'LocalPath'
              Width = 100
            end
            item
              Caption = 'UpdateFileMethod'
              Width = 100
            end
            item
              Caption = 'RemotePath'
              Width = 100
            end
            item
              Caption = 'UpdateRelative'
              Width = 100
            end>
          ReadOnly = True
          RowSelect = True
          TabOrder = 1
          ViewStyle = vsReport
          OnColumnClick = lvMakeSaveFileColumnClick
          OnDblClick = lvMakeSaveFileDblClick
          OnSelectItem = lvMakeSaveFileSelectItem
        end
        object btnMakeDelFile: TButton
          Left = 464
          Top = 16
          Width = 89
          Height = 17
          Caption = #21024#38500#25991#20214'(&D)'
          Enabled = False
          TabOrder = 2
          OnClick = btnMakeDelFileClick
        end
        object btnMakeModFile: TButton
          Left = 360
          Top = 16
          Width = 89
          Height = 17
          Caption = #20462#25913#25991#20214'(&M)'
          Enabled = False
          TabOrder = 3
          OnClick = btnMakeModFileClick
        end
      end
      object gbMakeSaveDir: TGroupBox
        Left = 8
        Top = 304
        Width = 369
        Height = 49
        Caption = #21319#32423#25991#20214#20445#23384#36335#24452
        TabOrder = 4
        object txtMakeSaveDir: TEdit
          Left = 8
          Top = 16
          Width = 305
          Height = 20
          ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
          TabOrder = 0
          OnChange = txtMakeTextChange
        end
        object btnMakeSaveDir: TButton
          Left = 328
          Top = 16
          Width = 25
          Height = 20
          Caption = '...'
          TabOrder = 1
          OnClick = btnMakeSaveDirClick
        end
      end
      object btnMakeRun: TButton
        Left = 504
        Top = 327
        Width = 75
        Height = 23
        Caption = #25191#34892'(&R)'
        Enabled = False
        TabOrder = 5
        OnClick = btnMakeRunClick
      end
      object btnMakeClear: TButton
        Left = 502
        Top = 7
        Width = 75
        Height = 23
        Caption = #28165#38500'(&C)'
        TabOrder = 6
        OnClick = btnMakeClearClick
      end
    end
  end
  object XPManifest1: TXPManifest
    Left = 568
    Top = 8
  end
  object ADOConn: TADOConnection
    ConnectionString = 
      'Provider=Microsoft.Jet.OLEDB.4.0;Data Source=PackDB.mdb;Persist ' +
      'Security Info=False'
    Provider = 'Microsoft.Jet.OLEDB.4.0'
    Left = 516
    Top = 7
  end
  object ADOCommand: TADOCommand
    Connection = ADOConn
    Parameters = <>
    Left = 468
    Top = 7
  end
  object dlgMakeOpen: TOpenDialog
    Filter = #25152#26377#25991#20214'(*.*)|*.*'
    Options = [ofHideReadOnly, ofAllowMultiSelect, ofEnableSizing]
    Left = 544
    Top = 72
  end
end
